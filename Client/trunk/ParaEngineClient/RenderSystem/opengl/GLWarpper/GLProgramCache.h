#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "GLType.h"

namespace ParaEngine
{
	class GLProgram;
	using GLProgramPtr = std::shared_ptr<GLProgram>;

	/**
	 * @addtogroup shaders
	 * @{
	 */

	 /** GLProgramCache
	  Singleton that stores manages GLProgram objects (shaders)
	  @since v2.0
	  */
	class GLProgramCache
	{
	public:
		/**
		 * @js ctor
		 */
		GLProgramCache();
		/**
		 * @js NA
		 * @lua NA
		 */
		~GLProgramCache();

		/** returns the shared instance */
		static GLProgramCache* getInstance();

		/** purges the cache. It releases the retained instance. */
		static void destroyInstance();

		/** @deprecated Use getInstance() instead */
		static GLProgramCache* sharedShaderCache();

		/** @deprecated Use destroyInstance() instead */
		static void purgeSharedShaderCache();

		/** loads the default shaders */
		void loadDefaultGLPrograms();
		void loadDefaultShaders() { loadDefaultGLPrograms(); }

		/** reload the default shaders */
		void reloadDefaultGLPrograms();
		void reloadDefaultShaders() { reloadDefaultGLPrograms(); }

		/** returns a GL program for a given key
		 */
		GLProgramPtr getGLProgram(const std::string &key);
		GLProgramPtr getProgram(const std::string &key) { return getGLProgram(key); }
		GLProgramPtr programForKey(const std::string &key) { return getGLProgram(key); }

		/** adds a GLProgram to the cache for a given name */
		void addGLProgram(GLProgramPtr program, const std::string &key);
		void addProgram(GLProgramPtr program, const std::string &key) { addGLProgram(program, key); }

	private:
		bool init();
		void loadDefaultGLProgram(GLProgram *program, int type);

		//    Dictionary* _programs;
		std::unordered_map<std::string, GLProgramPtr> _programs;
	};
	using GLProgramCachePtr = std::shared_ptr<GLProgramCache>;
}