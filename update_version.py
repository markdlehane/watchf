# Utility which updates a build number within source files.
#
import argparse

# Script build version number.
C_VERSION:str = '0.1a'

# Key Data.
C_SHORT:str = 'short'
C_LONG:str = 'long'
C_HELP:str = 'help'
C_ACTION:str = 'action'
C_TYPE:str = 'type'
C_DEFAULT:str = 'default'

# Argument data.
C_ARGUMENTS = [
    { C_SHORT:'-v', C_LONG:'--version', C_HELP:'Report the script version number.', C_ACTION:'store_true', C_TYPE:'option', },
    { C_SHORT:'-l', C_LONG:'--loud', C_HELP:'Verbosely report actions', C_ACTION:'store_true', C_TYPE:'option', },
    { C_SHORT:'-m', C_LONG:'--major', C_HELP:'Update the major build number', C_ACTION:'store_true', C_TYPE:'option', },    
    { C_SHORT:'-u', C_LONG:'--upset', C_HELP:'Update the major build number and reset the minor number', C_ACTION:'store_true', C_TYPE:'option', },
    { C_LONG:'file', C_HELP:'The name of the file to update.', C_TYPE:'param', C_DEFAULT:''},
]

def update_file(file_name:str, update_major:bool=False, reset_minor:bool=False) -> bool:
    import re
    ret = False
    try:
        source:list[str] = []
        with open(file=file_name, mode="rt") as f:
            for line in f:
                if ret == False:
                    pos:int = str(line).find("BUILD_NO")
                    if pos != -1:
                        part = line[8+pos:]
                        build_no = re.search(r'\s=\s"(\d+)\.(\d+)"(.+)', part)
                        if build_no != None and len(build_no.groups()) == 3:
                            major_no:int = int(build_no.group(1))
                            minor_no:int = int(build_no.group(2))
                            
                            # Update the build no.
                            if reset_minor:
                                minor_no = 0
                            else:
                                minor_no += 1
                            if update_major:
                                major_no += 1
                            
                            new_line:str = line[:8+pos] + f' = "{major_no}.{minor_no}"{build_no.group(3)}\n'
                            line = new_line
                            ret = True

                # Append the line to the output.
                source.append(line)
            f.close()

        # If an update has occurred then write the new file.
        if ret == True:
            ret = False # In case an error occurrs.
            with open(file=file_name, mode="wt") as f:
                for line in source:
                    f.write(line)
                f.flush()
                f.close()
                ret = True
            
    except FileNotFoundError:
        print(f'Could not open source file:{file_name}')
        ret = False
    
    return ret

# Argument compiler.
def build_arguments(parser:argparse.ArgumentParser, required_as_optional:bool=False) -> argparse.ArgumentParser:
    """ Construct the standard argument.

    Args:
        parser (argparse.ArgumentParser): An argument parser object.
        required_as_optional (bool, optional): if True, requried parameters are optional. Defaults to False.

    Returns:
        argparse.ArgumentParser: The argument parser object.
    """
    for option in C_ARGUMENTS:
        if option['type'] == 'option':
            parser.add_argument(option[C_SHORT], option[C_LONG], help=option[C_HELP], action=option[C_ACTION])
        
        else:
            # Work out the default value.
            default_val = None if option[C_DEFAULT] == '' else option[C_DEFAULT] 

            # Add the next argument depending on the optional_n
            if required_as_optional:
                # The project and location values are OPTIONAL.
                parser.add_argument(option[C_LONG], help=option[C_HELP], nargs='?', default=default_val)
            else:
                # The project and location values are REQUIRED.
                parser.add_argument(option[C_LONG], help=option[C_HELP], type=str, default=default_val)

    return parser

# First step - identifiy if the "version" option has been requested. If so just print the
# version number and quit.
try:
    parser = build_arguments(argparse.ArgumentParser(), required_as_optional=True)
    args = parser.parse_args()
    if args.version:
        print(f'Project generator version {C_VERSION}')
    else:
        # The version number was not requested so we build and extract the remain options.
        parser = build_arguments(argparse.ArgumentParser(), False)
        args = parser.parse_args()

        if args.version:
            print(f'Project generator version {C_VERSION}')
        
        else:
            if args.upset:
                update_file(file_name=args.file, update_major=True, reset_minor=True)
            else:
                update_file(file_name=args.file, update_major=args.major, reset_minor=False)
        
except SystemExit as e:
    # parser.parse_args() throws a SystemExit exception if it reaches the end of processing.
    print(f'')

