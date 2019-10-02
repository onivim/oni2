FROM centos:7

RUN yum -y update

RUN yum -y install gcc-c++ make sudo
RUN curl -sL https://rpm.nodesource.com/setup_10.x | sudo -E bash -
RUN yum -y install nodejs npm coreutils grep tar sed gawk diffutils

RUN yum -y install libXt libSM libice ncurses-devel libacl-devel libxrandr-devel libxinerama-devel libxcursor-devel libxi-devel mesa-libEGL-devel ragel gtk3-devel perl-Digest-SHA bzip2 m4 patch which cmake3 git

RUN node -v
RUN npm -v

RUN npm install --global --unsafe-perm=true esy@0.5.8
