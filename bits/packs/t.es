let path = Path('est.pak')
// let desc = path.readString().match(/pack\(.*/m)[0].split(',')[1]
dump(path.readString().match(/pack\(.*, '(.*)'/m))
