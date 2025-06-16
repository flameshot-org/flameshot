pkgname=flameshot-git
_pkgname=flameshot
pkgver=r2022.277eb2f4
pkgrel=1
pkgdesc="Powerful yet simple to use screenshot software"
arch=('i686' 'x86_64' 'aarch64' 'armv7h')
url="https://github.com/flameshot-org/flameshot"
license=('GPL-3.0-or-later')
depends=('qt6-base' 'qt6-svg' 'hicolor-icon-theme' 'kguiaddons')
makedepends=('qt6-tools' 'cmake' 'ninja')
optdepends=(
    'gnome-shell-extension-appindicator: for system tray icon if you are using Gnome'
    'grim: for wlroots wayland support'
    'xdg-desktop-portal: for wayland support, you will need the implementation for your wayland desktop environment'
    'qt6-imageformats: for additional export image formats (e.g. tiff, webp, and more)'
)
provides=(flameshot)
conflicts=(flameshot)
source=()

prepare() {
    cp -R "${startdir}/" "${srcdir}/${_pkgname}/"
}

pkgver() {
    cd "${srcdir}/${_pkgname}"

    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cd "${srcdir}/${_pkgname}"

    cmake -GNinja -B build -S . \
        -DCMAKE_BUILD_TYPE=None \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DUSE_WAYLAND_CLIPBOARD=1 \
        -DDISABLE_UPDATE_CHECKER=1 \

    cmake --build build
}

package() {
    cd "${srcdir}/${_pkgname}"

    DESTDIR="${pkgdir}" cmake --install build
}
