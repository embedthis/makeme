Expansive.load({
    meta: {
        title:       'Embedthis MakeMe Documentation',
        url:         'https://embedthis.com/esp/doc/',
        description: 'Embedthis MakeMe -- Fast, modern replacement for make and autoconf.',
        keywords:    'MakeMe, Bit, GYP, autoconf, make, cake',
    },
    expansive: {
        copy:    [ 'images' ],
        dependencies: { 'css/all.css.less': 'css/*.inc.less' },
        documents: [ '**', '!css/*.inc.less' ],
        plugins: [ 'less' ],
    }
})
