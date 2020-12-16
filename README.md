# itunes-cli
Command line interface for MacOS

# Motivation
Being on a mac, I always had a lot of trouble getting the [ncmpcpp](https://github.com/ncmpcpp/ncmpcpp) player to work for me. So I decided to create my own.

# What it Looks Like:
<img width="1440" alt="screenshot" src="https://user-images.githubusercontent.com/58267405/102382676-b2aa0d80-3f98-11eb-961d-abcfae0bdd2d.png">

# Build
1. Clone or download this project
2. Navigate to the project directory and run `make`
3. Run the executable `./itunes-cli`

# Features
+ Use the arrow keys to scroll through your library. 
+ Hit `enter` to play tracks and hit the `spacebar` to pause and resume them. 
+ Hit `q` to quit the client.

Easy as that!

# Known Issues
+ Pausing causes track to sound like a broken record.
+ Resuming doesn't pick up from where it left off. Might be a consequence of the previous bug.
+ Tracks containing non-ASCII encodings are not valid.
+ Resizing window causes things to either be cut off or not show at all.

# TODO
+ Work on above bugs.
+ Get lyrics for songs.
+ Add a shuffle feature.
