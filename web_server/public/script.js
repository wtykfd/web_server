document.addEventListener('DOMContentLoaded', function() {
    // 检查用户是否已登录
    checkAuthStatus();
    
    // 公共API按钮事件
    document.getElementById('public-btn').addEventListener('click', function() {
        fetch('/api/public')
            .then(response => response.json())
            .then(data => {
                document.getElementById('result').innerHTML = 
                    `<pre>${JSON.stringify(data, null, 2)}</pre>`;
            })
            .catch(error => {
                document.getElementById('result').innerHTML = 
                    `<p class="error">Error: ${error.message}</p>`;
            });
    });
    
    // 受保护API按钮事件
    document.getElementById('protected-btn').addEventListener('click', function() {
        fetch('/api/protected', {
            credentials: 'include' // 包含Cookie
        })
            .then(response => {
                if (response.status === 401 || response.status === 403) {
                    window.location.href = '/login';
                    throw new Error('Not authenticated');
                }
                return response.json();
            })
            .then(data => {
                document.getElementById('result').innerHTML = 
                    `<pre>${JSON.stringify(data, null, 2)}</pre>`;
            })
            .catch(error => {
                document.getElementById('result').innerHTML = 
                    `<p class="error">Error: ${error.message}</p>`;
            });
    });
    
    // 用户API按钮事件
    document.getElementById('user-btn').addEventListener('click', function() {
        fetch('/api/users/123', {
            credentials: 'include'
        })
            .then(response => {
                if (response.status === 401 || response.status === 403) {
                    window.location.href = '/login';
                    throw new Error('Not authenticated');
                }
                return response.json();
            })
            .then(data => {
                document.getElementById('result').innerHTML = 
                    `<pre>${JSON.stringify(data, null, 2)}</pre>`;
            })
            .catch(error => {
                document.getElementById('result').innerHTML = 
                    `<p class="error">Error: ${error.message}</p>`;
            });
    });
});

// 检查认证状态
function checkAuthStatus() {
    fetch('/api/protected', {
        credentials: 'include',
        method: 'HEAD'
    })
    .then(response => {
        if (response.status === 200) {
            // 用户已登录
            document.getElementById('logout-btn').style.display = 'inline';
            document.getElementById('login-btn').style.display = 'none';
        } else {
            // 用户未登录
            document.getElementById('logout-btn').style.display = 'none';
            document.getElementById('login-btn').style.display = 'inline';
        }
    })
    .catch(error => {
        console.error('Auth check failed:', error);
    });
}