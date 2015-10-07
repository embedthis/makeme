let text = '<script>some text</script>
<div>outer<p>inner</p></div>
<script>other text</script>'

let re = /<([\w_\-:.]+)>(.*)<\/\1>/g

dump(re.exec(text))
