FROM centos:6

RUN yum -y update

RUN yum -y install nodejs npm gcc coreutils grep tar sed gawk make diffutils

RUN node --version
RUN npm --version

RUN npm install --global --unsafe-perm=true esy@0.5.8
