
#ifndef __CCGLPROGRAMCACHE_H__
#define __CCGLPROGRAMCACHE_H__

#include <string>
#include <unordered_map>

#include "CCType.h"

NS_CC_BEGIN

class GLProgram;

/**
 * @addtogroup shaders
 * @{
 */

/** GLProgramCache
 Singleton that stores manages GLProgram objects (shaders)
 @since v2.0
 */
class GLProgramCache : public Ref
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
    GLProgram * getGLProgram(const std::string &key);
    GLProgram * getProgram(const std::string &key) { return getGLProgram(key); }
    GLProgram * programForKey(const std::string &key){ return getGLProgram(key); }

    /** adds a GLProgram to the cache for a given name */
    void addGLProgram(GLProgram* program, const std::string &key);
    void addProgram(GLProgram* program, const std::string &key) { addGLProgram(program, key); }

private:
    bool init();
    void loadDefaultGLProgram(GLProgram *program, int type);

//    Dictionary* _programs;
    std::unordered_map<std::string, GLProgram*> _programs;
};

// end of shaders group
/// @}

NS_CC_END

#endif /* __CCGLPROGRAMCACHE_H__ */
