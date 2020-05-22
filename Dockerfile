FROM ubuntu:16.04
MAINTAINER xuntian "li.zq@foxmail.com"

ADD ./ /opt/NPLRuntime 

WORKDIR /opt/NPLRuntime

#RUN git submodule init && git submodule update
ARG JOBS
ARG CMAKE_BUILD_TYPE
RUN chmod +x npl_install.sh && ./npl_install.sh ${JOBS:-4} ${CMAKE_BUILD_TYPE:-Release} && rm -rf /opt/NPLRuntime/bin && rm -rf /usr/local/include/boost

#RUN rm /opt/NPLRuntime/ParaWorld/bin64/liblua.so && cp /opt/NPLRuntime/ParaWorld/bin64/liblua51.so /opt/NPLRuntime/ParaWorld/bin64/liblua.so

#RUN cd /opt && git clone https://github.com/LiXizhi/NPLRuntime.git && \
#        cd /opt/NPLRuntime && git submodule init && git submodule update && \
#        chmod +x npl_install.sh; sync; ./npl_install.sh && rm -rf /opt/NPLRuntime/bin && rm -rf /usr/local/include/boost
