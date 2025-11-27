window.onload = function () {
    console.log('HTML documentation ready');

    if (typeof app !== 'undefined') {
        // 测试原有的JS调用C++功能
        app.call("hello", ["cef"])
            .then(result => {
                console.log('JS调用C++结果:', result);

                // 在页面上显示JS调用C++的结果
                const jsDisplayDiv = document.getElementById('js-call-display');
                if (jsDisplayDiv) {
                    jsDisplayDiv.innerHTML = `
                        <strong>JS调用C++成功!</strong><br>
                        <strong>结果:</strong> ${result}<br>
                        <strong>时间:</strong> ${new Date().toLocaleString()}
                    `;
                }
            })
            .catch(error => {
                console.error('JS调用C++失败:', error);

                const jsDisplayDiv = document.getElementById('js-call-display');
                if (jsDisplayDiv) {
                    jsDisplayDiv.innerHTML = `
                        <strong>JS调用C++失败!</strong><br>
                        <strong>错误:</strong> ${error}<br>
                        <strong>时间:</strong> ${new Date().toLocaleString()}
                    `;
                }
            });

        // 注册C++调用JS的事件监听器
        const onCppEvent = function(...args) {
            console.log('C++调用JS事件，参数:', args);

            // 在页面上显示C++调用
            const displayDiv = document.getElementById('cpp-call-display');
            if (displayDiv) {
                displayDiv.innerHTML = `
                    <strong>✓ 收到C++主动调用!</strong><br>
                    <strong>事件名称:</strong> cppEvent<br>
                    <strong>参数:</strong> ${args.join(', ')}<br>
                    <strong>时间:</strong> ${new Date().toLocaleString()}
                `;
            }

            // 可选：显示一个通知
            showNotification('收到来自C++的调用: ' + args.join(', '));
        };

        // 注册事件监听器
        const success = app.addEventListener("cppEvent", onCppEvent);
        if (success) {
            console.log('✓ C++调用JS事件监听器注册成功');
        } else {
            console.error('✗ C++调用JS事件监听器注册失败');
        }

        // 添加一个状态更新
        setTimeout(() => {
            const cppDisplayDiv = document.getElementById('cpp-call-display');
            if (cppDisplayDiv) {
                cppDisplayDiv.innerHTML = '<strong>⏳ 等待C++主动调用JS... (3秒后)</strong>';
            }
        }, 1000);

        console.log('✓ JS环境初始化完成，等待C++调用...');

    } else {
        console.error('✗ app object is not defined and the V8 extension may not load correctly');

        const jsDisplayDiv = document.getElementById('js-call-display');
        if (jsDisplayDiv) {
            jsDisplayDiv.innerHTML = '<strong>❌ app对象未定义，V8扩展可能加载失败</strong>';
        }
    }
};

// 显示通知的辅助函数
function showNotification(message) {
    // 创建一个简单的通知元素
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        background: #4caf50;
        color: white;
        padding: 10px 20px;
        border-radius: 5px;
        box-shadow: 0 2px 10px rgba(0,0,0,0.2);
        z-index: 1000;
        font-family: Arial, sans-serif;
    `;
    notification.textContent = message;

    document.body.appendChild(notification);

    // 3秒后移除通知
    setTimeout(() => {
        if (notification.parentNode) {
            notification.parentNode.removeChild(notification);
        }
    }, 3000);
}