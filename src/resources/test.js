window.onload = function () {
    console.log('HTML documentation ready');

    if (typeof app !== 'undefined') {
        console.log(app);
        console.log(app.call("test", [1, "2", true]))
    } else {
        console.error('app object is not defined and the V8 extension may not load correctly');
    }
}