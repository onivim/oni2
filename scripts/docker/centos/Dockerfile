FROM centos:7

RUN yum -y update

RUN yum -y install gcc-c++ make sudo
RUN curl -sL https://rpm.nodesource.com/setup_10.x | sudo -E bash -
RUN yum -y install nodejs npm coreutils grep tar sed gawk diffutils

RUN yum -y install ragel libXt-devel libSM-devel libICE-devel ncurses-devel libacl-devel libxrandr-devel libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel mesa-libGLU-devel ragel gtk3-devel perl-Digest-SHA bzip2 m4 patch which cmake3 git

RUN rpm -i https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/colm-0.13.0.4-2.el7.x86_64.rpm
RUN rpm -i https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/r/ragel-7.0.0.9-2.el7.x86_64.rpm

RUN node -v
RUN npm -v

RUN npm install --global --unsafe-perm=true esy@0.5.8
