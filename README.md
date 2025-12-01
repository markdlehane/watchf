# WatchFile
> Execute a command when a file is updated or directory content changes.

![Static Badge](https://img.shields.io/badge/GNU_C-11.4.0-blue?style=flat&logo=gnu&logoColor=white&logoSize=auto)
![Static Badge](https://img.shields.io/badge/CMake-3.18-green?style=flat&logo=cmake&logoColor=orange&logoSize=auto)

This tool enables the constant monitoring of resources in order to automate tasks such as compilation, installation, update and more.

Originally put together as a fun way to refresh my knowledge of signals and inotify, this became a thoroughly useful tool when working
with sass, then automated testing and has now become a staple in my project toolkit. 

# Installation

On *nix like systems it can be simply placed in a suitable folder on the path or, for example in the home directory's .local/bin:

```sh
cp watchf ~/.local/bin/
```

* The VsCode project provided supplies an installation task that does exactly this. There is also a bash script to simplify the use of the tool 
as a background process. The installation task is aware that such background tasks may prevent installation after an update and this offers
the option of killing processes that are running using the watchf executable beforehand.

* Additionally, the tool is supplied with a bash script which runs the watcher task in the background:-

```bash
watch filename/directoryname "command"
```

This command is a simple script which uses the *nix **nohup** [^1] comnand to move the watcher job to a background task, as follows:-

```bash
#!/bin/bash
nohup watchf -f $1 -e "sassc $1 $2" > /dev/null 2>&1 &
status=$?
[ $status -eq 0 ] && echo "Successfully Watching $1" || echo "Failed to watch $cmd."
```

The status check lines may be omitted if you require no output at all. This script must be manually installed.

## Usage example



A few motivating and useful examples of how your product can be used. Spice this up with code blocks and potentially more screenshots.

#### To monitor a simple sass file, compiling on changes:-

```
watchf -f filename.scss -e "sassc filename.scss ../public/assets/filename.css"

nohuo watchf -f filename.scss -e "sassc filename.scss ../public/assets/filename.css" > /dev/null 2>&1 &
```

#### To monitor a directory and compile the file that changes.

```
watchf -f sass/*.scss -e sassc $1.scss ../public/assets/$1.css
```


## Development setup

For this project I used VsCode on any *nix environment (including WSL2 on Windows). The extensions requried are as follows:

| Name| Manufactuer | Description |
| ----------- | ----------- | ----------- |
| CMake Tools | Microsoft | Extended CMake support in Visual Studio Code |
| C/C++ Extension Pack| Microsoft | Popular extensions for C++ development in Visual Studio Code |

## Release History

* 0.1.0 (build 141).
    * The first proper release.
* 0.0.1
    * Work in progress.

## Meta

Distributed under the MIT license. See [LICENCE](LICENCE) for more information.

[https://github.com/markdlehane](https://github.com/markdlehane/watchf)

## References

[^1]: See man page nohup(1) (man 1 nohup).
