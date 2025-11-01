FROM rockylinux:9

ENV DNF_YES=1

RUN dnf -y update && \
    dnf -y install \
      epel-release \
      dnf-plugins-core && \
    dnf -y install \
      make cmake gcc-c++ git which pkgconfig \
      qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
    || true && dnf clean all

WORKDIR /work

CMD ["/bin/bash"]
