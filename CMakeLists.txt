# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.22
# Note: 
# make sure you can build the ParaEngine Server before you build ParaEngineClient. See the cmake file in the Server root trunk. 


cmake_minimum_required (VERSION 2.6)

project (ParaEngine)

# the client will include the server file. 
add_subdirectory (Client)

