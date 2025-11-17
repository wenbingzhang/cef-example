window.onload = function () {
    console.log('HTML documentation ready');

    if (typeof app !== 'undefined') {
        app.call("hello", ["cef"])
            .then(result => {
                console.log('Promise resolved:', result);
            })
            .catch(error => {
                console.error('Promise rejected:', error);
            });

    } else {
        console.error('app object is not defined and the V8 extension may not load correctly');
    }
}