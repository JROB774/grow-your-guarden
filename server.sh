cd "binary/web"
dir=$(pwd)
osascript -e "tell app \"Terminal\" to do script \"cd $dir; python3 -m http.server\""
