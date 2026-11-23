// /temp also holds the file_packager's boot files. Mount before preRun, then restore
// user changes over those defaults in the same FS.syncfs(true) as worlds and apps.
addToLibrary({
  $NPLTempFS__deps: ['$FS', '$MEMFS', '$IDBFS'],
  $NPLTempFS__postset: "if (typeof ENVIRONMENT_IS_PTHREAD === 'undefined' || !ENVIRONMENT_IS_PTHREAD) { FS.mkdirTree('/temp'); FS.mount(NPLTempFS, {}, '/temp'); }",
  $NPLTempFS: {
    // CAssetManifest registers only payloads it actually uses. A caller can
    // write its own hash-named files alongside them with ordinary ParaIO.
    excludeAsset(path) {
      var mount = FS.lookupPath('/temp').node.mount;
      if (mount.assets.has(path)) return;
      mount.assets.add(path);
      var node = FS.analyzePath(path, true).object;
      if (node && node.savedInTemp) {
        mount.dirty.add(path);
        mount.scheduleSave();
      }
    },

    mount(mount) {
      var root = MEMFS.mount(mount);
      var dirty = mount.dirty = new Set();
      mount.defaults = new Set();
      mount.assets = new Set();
      mount.work = Promise.resolve();

      function changed(path) {
        if (mount.restoring || mount.assets.has(path)) return;
        if (!mount.ready) {
          mount.defaults.add(path);
          return;
        }
        dirty.add(path);
        scheduleSave();
      }
      function scheduleSave() {
        if (mount.scheduled) return;
        mount.scheduled = true;
        NPLTempFS.syncfs(mount, false, err => {
          mount.scheduled = false;
          if (err) console.error('Could not save /temp', err);
          else if (dirty.size) scheduleSave();
        });
      }
      function changedTree(node, path) {
        changed(path);
        if (FS.isDir(node.mode) && !node.mounted) {
          for (var child of Object.values(node.contents)) {
            changedTree(child, path + '/' + child.name);
          }
        }
      }
      var ops = MEMFS.node_ops;
      function setattr(node, attr) {
        ops.setattr(node, attr);
        if (attr.size !== undefined || attr.mode !== undefined || attr.mtime !== undefined) {
          changed(FS.getPath(node));
        }
      }
      var directoryOps = Object.assign(Object.create(root.node_ops), {setattr,
        mknod(parent, name, mode, dev) {
          var node = ops.mknod(parent, name, mode, dev);
          if (FS.isDir(mode)) node.node_ops = directoryOps;
          else if (FS.isFile(mode)) {
            node.node_ops = Object.assign(Object.create(node.node_ops), {setattr});
            node.stream_ops = fileStreams;
          }
          changed(FS.getPath(node));
          return node;
        },
        symlink(parent, name, target) {
          var node = ops.symlink(parent, name, target);
          changed(FS.getPath(node));
          return node;
        },
        rename(node, parent, name) {
          var oldPath = FS.getPath(node);
          ops.rename(node, parent, name);
          changedTree(node, oldPath);
          changedTree(node, FS.getPath(parent) + '/' + name);
        },
        unlink(parent, name) {
          ops.unlink(parent, name);
          changed(FS.getPath(parent) + '/' + name);
        },
        rmdir(parent, name) {
          ops.rmdir(parent, name);
          changed(FS.getPath(parent) + '/' + name);
        }
      });
      var fileStreams = Object.assign(Object.create(MEMFS.ops_table.file.stream), {
        write(stream, ...args) {
          var written = MEMFS.stream_ops.write(stream, ...args);
          if (written) changed(FS.getPath(stream.node));
          return written;
        },
        msync(stream, ...args) {
          var result = MEMFS.stream_ops.msync(stream, ...args);
          changed(FS.getPath(stream.node));
          return result;
        }
      });
      mount.scheduleSave = scheduleSave;
      root.node_ops = directoryOps;
      return root;
    },

    // Serialize explicit and automatic syncs. Update only touched keys, so a
    // write from another game tab is not removed by an older in-memory tree.
    syncfs(mount, populate, callback) {
      var work = mount.work.then(() => new Promise((resolve, reject) => {
        if (!populate && (!mount.ready || !mount.dirty.size)) return resolve();
        IDBFS.getDB(mount.mountpoint, (err, db) => {
          if (err) return reject(err);
          try {
            if (populate) NPLTempFS.restore(mount, db, resolve, reject);
            else NPLTempFS.save(mount, db, resolve, reject);
          } catch (error) { reject(error); }
        });
      }));
      mount.work = work.catch(() => {});
      work.then(() => callback(null), callback);
    },

    restore(mount, db, resolve, reject) {
      var tx = db.transaction('FILE_DATA', 'readonly');
      var request = tx.objectStore('FILE_DATA').openCursor();
      mount.restoring = true;
      tx.onabort = () => { mount.restoring = false; reject(tx.error); };
      request.onsuccess = () => {
        var cursor = request.result;
        try {
          if (!cursor) return;
          var path = cursor.key;
          // The Haqi shell's existing wardrobe mount owns its own database.
          var nested = mount.mounts.some(m => path === m.mountpoint || path.startsWith(m.mountpoint + '/'));
          if (path.startsWith('/temp/') && !nested && !mount.assets.has(path)) {
            var entry = cursor.value;
            if (entry.deleted) {
              if (FS.analyzePath(path, true).exists) removeTree(path);
            } else {
              // The bundle can drop a default directory that now holds player files.
              FS.mkdirTree(path.slice(0, path.lastIndexOf('/')));
              // A new data bundle can ship a different node type at the old path.
              var previous = FS.analyzePath(path, true).object;
              if (previous && (FS.isLink(previous.mode) ||
                  FS.isDir(previous.mode) !== FS.isDir(entry.mode) || FS.isLink(entry.mode))) {
                removeTree(path);
              }
              // IDBFS.storeLocalEntry calls chmod on the target of a link, which
              // can be dangling or sorted after the link in the database.
              if (FS.isLink(entry.mode)) FS.symlink(entry.link, path);
              else IDBFS.storeLocalEntry(path, entry, fail);
              FS.lookupPath(path).node.savedInTemp = true;
            }
          }
          cursor.continue();
        } catch (error) {
          tx.abort();
          reject(new Error('Could not restore ' + path + ': ' + error));
        }
      };
      function removeTree(path) {
        var node = FS.lookupPath(path).node;
        if (node.mount !== mount) throw new Error('Cannot replace mounted directory ' + path);
        if (FS.isDir(node.mode)) {
          for (var name of FS.readdir(path)) {
            if (name !== '.' && name !== '..') removeTree(path + '/' + name);
          }
        }
        IDBFS.removeLocalEntry(path, fail);
      }
      function fail(error) {
        if (error) throw error;
      }
      tx.oncomplete = () => { mount.restoring = false; mount.ready = true; resolve(); };
    },

    save(mount, db, resolve, reject) {
      var paths = Array.from(mount.dirty);
      var tx = db.transaction('FILE_DATA', 'readwrite');
      var store = tx.objectStore('FILE_DATA');
      mount.dirty.clear();
      function retry(error) {
        for (var path of paths) mount.dirty.add(path);
        reject(error);
      }
      tx.onabort = () => retry(tx.error);
      tx.oncomplete = resolve;
      try {
        for (var path of paths) {
          var node = FS.analyzePath(path, true).object;
          if (mount.assets.has(path)) {
            store.delete(path);
          } else if (node && node.mount === mount) {
            IDBFS.loadLocalEntry(path, (err, entry) => {
              if (err) throw err;
              store.put(entry, path);
              node.savedInTemp = true;
            });
          } else if (!node) {
            // A packaged default is recreated on every boot unless its deletion is recorded.
            if (mount.defaults.has(path)) store.put({deleted: true}, path);
            else store.delete(path);
          }
        }
      } catch (error) { tx.abort(); retry(error); }
    }
  },
  npl_exclude_manifest_file__deps: ['$NPLTempFS', '$UTF8ToString'],
  npl_exclude_manifest_file__proxy: 'sync',
  npl_exclude_manifest_file: path => NPLTempFS.excludeAsset('/' + UTF8ToString(path)),
});
