

#ifdef PLATFORM_MAC



bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray)
{
    return true;
}

bool GLProgram::initWithByteArrays(const GLchar* vShaderByteArray, const GLchar* fShaderByteArray, const std::string& compileTimeDefines)
{
    return true;
}

bool GLProgram::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename)
{
    return true;
}

bool GLProgram::initWithFilenames(const std::string& vShaderFilename, const std::string& fShaderFilename, const std::string& compileTimeDefines)
{
    return true;
}


#endif
