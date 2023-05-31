# Ultimaker 3 Tools
Command line tools for Ultimaker 3. 64-bit binaries available for Windows and Ubuntu.

## Installation
For Windows, extract the .zip file, then run `um3tools.exe`.

For Ubuntu, install the .deb file: `sudo apt install ./um3tools_1.1_amd64.deb`. Then the `um3tools` command will be available.

## Usage
```bash
um3tools [--help] [--version] [--credentials PATH] [--id VALUE] [--key VALUE] {authenticate,music,timelapse}

Optional arguments:
  -h, --help                    shows help message and exits 
  -v, --version                 prints version information and exits 
  -c, --credentials PATH        Path to file containing authentication credentials with the ID on the first line, and key on the second. Required for all POST requests. 
  -id, --id VALUE               Authentication ID. 
  -k, --key VALUE               Authentication key. 

Subcommands:
  authenticate           Generate authentication credentials for a printer. Requires physical access to the printer.
  music                  Plays a melody from a MusicXML file using the printer beep.
  timelapse              Records a timelapse of the current print job. Requires ffmpeg to be installed.
```

### Timelapse
```bash
um3tools timelapse [--help] [--out-file PATH] [--frame-rate RATE] printer-address

Records a timelapse of the current print job. Requires ffmpeg to be installed.

Positional arguments:
  printer-address       IP or network address of the printer. 

Optional arguments:
  -h, --help            shows help message and exits 
  --out-file PATH       Path to timelapse video output file. 
  -r, --frame-rate RATE Frame rate of timelapse video in frames per second (integer). [default: 25]
```

### Authenticator
```bash
um3tools authenticate [--help] [--output PATH] printer-address username

Generate authentication credentials for a printer. Requires physical access to the printer.

Positional arguments:
  printer-address       IP or network address of the printer. 
  username              Username to be associated with your authentication credentials. 

Optional arguments:
  -h, --help            shows help message and exits 
  --output PATH         Path to a file to store credentials. 
```

### MusicXML player
```bash
um3tools music [--help] printer-address xml-path

Plays a melody from a MusicXML file using the printer beep.

Positional arguments:
  printer-address       IP or network address of the printer. 
  xml-path              Path to MusicXML (uncompressed, .xml) file to play. Only the notes in the first 'part' will be played. 

Optional arguments:
  -h, --help            shows help message and exits 
```
