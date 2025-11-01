FROM opensuse/tumbleweed:latest

RUN zypper --non-interactive refresh && \
    zypper --non-interactive install -t pattern devel_C_C++ && \
    zypper --non-interactive install cmake gcc-c++ make git pkg-config && \
    zypper --non-interactive install \
      libQt6-qtbase-devel libQt6-qtsvg-devel libQt6-qttools-devel libQt6-linguist-devel || true && \
    zypper clean -a

WORKDIR /work

CMD ["/bin/bash"]
