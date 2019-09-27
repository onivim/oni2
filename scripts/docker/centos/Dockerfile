FROM centos:6

RUN yum -y update

RUN yum -y install gcc-c++ make
RUN curl -sL https://rpm.nodesource.com/setup_10.x | sudo -E bash -
RUN yum -y install nodejs npm coreutils grep tar sed gawk diffutils

RUN yum -y libacl1-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libegl1-mesa-dev mesa-utils mesa-utils-extra ragel libgtk-3-dev

RUN node -v
RUN npm -v

RUN npm install --global --unsafe-perm=true esy@0.5.8
