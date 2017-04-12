# ubuntu-nplruntime by 2017-04-12
FROM ubuntu:latest
MAINTAINER xuntian "li.zq@foxmail.com"

# 1
RUN apt-get -y update && apt-get -y install git wget vim build-essential cmake libcurl4-openssl-dev \
	libssl-dev libssh2-1-dev libbz2-dev freeglut3 freeglut3-dev libglew1.5 libglew1.5-dev \
	libglu1-mesa libglu1-mesa-dev libgl1-mesa-glx libgl1-mesa-dev

# 2 
RUN cd /opt && git clone https://github.com/LiXizhi/NPLRuntime.git && \
	cd /opt/NPLRuntime && git submodule init && git submodule update

# 3
RUN cd /opt/NPLRuntime && chmod +x npl_boost_install.sh; sync; ./npl_boost_install.sh

# 4
RUN cd /opt/NPLRuntime && chmod +x build_linux.sh; sync; ./build_linux.sh