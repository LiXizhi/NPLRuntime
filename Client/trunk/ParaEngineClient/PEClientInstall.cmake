# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.25

ADD_CUSTOM_COMMAND(
   TARGET ParaEngineClient
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineClient> ${OUTPUT_BIN_DIR}
)

