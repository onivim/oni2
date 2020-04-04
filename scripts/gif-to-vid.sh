
ffmpeg -i "$1" -b:v 0 -crf 25 -f mp4 -vcodec libx264 -pix_fmt yuv420p "$2.mp4"
ffmpeg -i "$1" -c vp9 -b:v 0 -crf 41 "$2.webm"
