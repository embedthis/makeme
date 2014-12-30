Expansive.load({
    meta: {
        title:       'Embedthis MakeMe Documentation',
        url:         'https://embedthis.com/esp/doc/',
        description: 'Embedthis MakeMe -- Fast, modern replacement for make and autoconf.',
    },
    expansive: {
        copy:    [ 'images' ],
        dependencies: { 'css/all.css.less': '**.less' },
        documents: [ '**', '!**.less', '**.css.less' ],
        plugins: [ 'less' ],
    }
})
