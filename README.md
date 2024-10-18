# ES CSP 1.6 Server

## Brief

This is an example CSP 1.6 server implementation used for transferring of files
and/or receiving data between the supported physical layers and a Endurosat's On-Board Computer.

## Scope

The application can be used on any Linux/Posix based machine that is able
to run [libcsp](https://github.com/libcsp/libcsp).

The provided server supports:
* `KISS` (UART)
* `CAN`  (optional)

This could be easily extended for other interfaces supported by `CSP` if necessary.

## Structure

    es-csp-server
              ├── app                      --> example csp server application
              ├── cmake                    --> helper CMake modules
              ├── CMakeLists.txt           --> repo CMakeLists.txt file
              ├── docs                     --> API documentation
              ├── external
              │   └── libcsp               --> actual CSP1.6 submodule
              ├── libs
              │   ├── es_csp_server        --> CSP server library
              │   ├── es_log               --> sample logging library
              │   ├── es_tftp              --> es tftp library
              │   └── libcsp
              │       ├── CMakeLists.txt
              │       └── patches          --> CMakeLists.txt file for CSP1.6
              └── README.md                --> this readme

## Requirements

Build:
* `git`  => any
* `CMake` >= 3.21
* `pkg-config` = any
* `gcc`  => 11 (for -fanalyzer)
* `ninja` = any (optional)
* `threads/pthreads` = any
* `libsocketcan` = any
* `clang-tidy` = any (optional)
* `cpp-check` = any (optional)
* `systemd`  => any (optional)

API docs python packages:
* `clang-17` (development)
* `sphinx-rtd-theme, sphinxcontrib-mermaid, furo, sphinx-copybutton,
  sphinx-design, sphinx-inline-tabs, sphinxcontrib.autoprogram, myst-parser,
  linuxdoc, sphinx_c_autodocm, sphinx_git, pygit2`

## Installation
```console
   ~$ git clone  https://github.com/endurosat/es-csp-server.git ${CSP_SERVER}
```

```console
   ~$ cd ${CSP_SERVER}
   ~$ rm -rf build && cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr  . -B build && cmake --build ./build
   ~$ cd ${CSP_SERVER}/build && ninja install
```

**_NOTE:_**  change the desired install prefix and build type;  <br>
**_NOTE:_**  for cross-compilation use CMake toolchain file or manually set your environment

## Using Clang
Pass `-DCMAKE_C_COMPILER=clang` to cmake or use global overrides / toolchain file.

## Running

* process in interactive shell:
```console
   /build/app$ ./csp_server --help
   arg parse
   Usage: csp_server [OPTION...]

     -d, --phy_device=device    Physical device endpoint
     -p, --phy_layer=physical-layer-type
                                Physical layer type [ uart | can ]
     -v, --verbose=verbose      Set verbosity level
     -?, --help                 Give this help list
         --usage                Give a short usage message

   Mandatory or optional arguments to long options are also mandatory or optional
   for any corresponding short options.
```

```console
   /build/app$ csp_server -d /dev/ttyUSB0 -p uart -v TRACE
    main [INFO] (98): CSP addr: 10 (0x0A), device:/dev/ttyUSB0 (uart)
    csp_server [DEBUG] (171): Creating new server
    csp_server [DEBUG] (246): UART iface init done
    es_tftp [DEBUG] (290): Creating new tftp context
    csp_server [DEBUG] (305): Starting server...
```

* sample systemd service for CAN:
```
    [Unit]
    Description=ES Proxy Service: CSP Server (can)
    BindsTo=sys-subsystem-net-devices-can0.device
    After=sys-subsystem-net-devices-can0.device

    [Service]
    Type=simple
    User=root
    Group=root
    ExecStart=/usr/bin/csp_server -d can0 -p can
    Restart=on-failure
    RestartSec=2
    RemainAfterExit=yes
    TimeoutStopSec=1

    [Install]
    WantedBy=multi-user.target
```
* sample systemd service for UART
```
    [Unit]
    Description=ES Proxy Service: CSP Server (uart)
    ConditionPathExists=/dev/ttyS0

    [Service]
    Type=simple
    User=root
    Group=root
    ExecStart=/usr/bin/csp_server -d /dev/ttyS0 -p uart
    Restart=on-failure
    RestartSec=2
    RemainAfterExit=yes
    TimeoutStopSec=1

    [Install]
    WantedBy=multi-user.target
```
# ES TFTP

The current `csp-server` demo supports file transfer over the simplified
`es-tftp` protocol.

The file transfer protocol is used to transfer files between the SDR module and
an external module. This section describes the protocol, how to use the API to
upload/download files and the limitations of it.

A file transfer is initiated with either write request (file upload) `WRQ` or
read request (file download) `RRQ`. The contents of the file are exchanged using
data packets with block size of `512` bytes. A data packet of less than block
size of `512` bytes signals termination of a transfer. Notice that both sides
involved in a transfer are considered senders and receivers. One sends data and
receives acknowledgments, the other sends acknowledgments and receives data.

An error cause termination of the connection. An error is signalled by sending
an error packet. This packet is not acknowledged.

Errors are caused by three types of events:
* not being able to satisfy the request (e.g., file not found, access violation)
* receiving a packet which cannot be decoded (e.g physical layer errors)
* losing access to a necessary resource (e.g., disk full or access denied during
  a transfer).

## API
The documentation for the module can be generated using sphinx in from the
`./docs` directory.

```console
   es-csp-server$ cd docs
   docs$ rm -rf build && cmake . -B build && cmake --build ./build
```

The resulting `rtd` based api documentation should be in `./build/html/index.html`

# Libcsp
You can define different build time options that affect `libcsp` behavior in
its corresponding CMakeLists.txt (libs/libcsp/patches/CMakeLists.txt) file.
This is necessary since the 1.6 CSP does not have native CMake support. Refer to
CSP's documentation in [libcsp](https://github.com/libcsp/libcsp).

# FAQs

**Q: What happens if we want to upload/download a file which already exists on the remote/local file system?**  <br>
**A:** The files are always truncated (deleted) in case they already exist on the file system.

**Q: Can commands be send when there is a file transfer in progress?**  <br>
**A:** Currently the csp server on the SDR doesn't support parallel execution of
commands while there is a file transfer in progress. If such use case is
required the file transfer on the SDR can be redesigned as an FSM which will
make possible this behaviour.

**Q: What are the buffer sizes in use?**  <br>
**A:** Currently:
```
ES_TFTP_BLOCK_SIZE = 256 //bytes
CSP_BUFFER_SIZE    = 256 //bytes
```

**Q: What are the used CSP address and application port?**  <br>
**A:** Currently:
```
ES_CSP_SERVER_ADDRESS_DFLT 10
ES_CSP_TFTP_PORT           11
```
# Software license
The source code is available under MIT license, see LICENSE for license text
