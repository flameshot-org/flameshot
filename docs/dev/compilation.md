## Compilation
### GUI
Just download QT Creator and QT5 and import the project selecting the `flameshot.pro` file. Hit the "Build" button.

### Debian
Dependencies:
````
apt install -y git g++ build-essential qt5-qmake qt5-default
````

Compilation: run `qmake && make` in the main directory.

### Fedora
Dependencies:
````
dnf install -y qt5-devel gcc-c++ git qt5-qtbase-devel
````

Compilation:  run `qmake-qt5 && make` in the main directory.

### Arch
Dependencies:
````
pacman -S git qt5-base base-devel
````

Compilation:  run `qmake && make` in the main directory.

## Install

After the compilation you only have to add the DBus related files to its respective directories

````
cp dbus/org.dharkael.Flameshot.xml /usr/share/dbus-1/interfaces/ && \
cp dbus/org.dharkael.Flameshot.service /usr/share/dbus-1/services/
````

Finally add the compilled binary to /usr/bin
