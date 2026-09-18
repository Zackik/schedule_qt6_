const http = require('http');

const PORT = process.env.PORT || 3000;

const server = http.createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
  res.end(`
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Smart Schedule - C++ Desktop App</title>
      <style>
        body {
          font-family: system-ui, -apple-system, sans-serif;
          display: flex;
          align-items: center;
          justify-content: center;
          height: 100vh;
          margin: 0;
          background-color: #f3f4f6;
          color: #1f2937;
          text-align: center;
          padding: 20px;
        }
        .container {
          background: white;
          padding: 40px;
          border-radius: 12px;
          box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
          max-width: 600px;
        }
        h1 { color: #2563eb; }
        p { line-height: 1.6; }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>🛠️ C++ Qt6 Desktop Application</h1>
        <p>Đây là một dự án phần mềm máy tính (Desktop App) thuần C++ và Qt6.</p>
        <p>Môi trường web này không thể chạy trực tiếp ứng dụng C++. Để sử dụng và biên dịch ứng dụng, vui lòng chọn <b>Export -> Download ZIP</b> ở góc trên cùng bên phải màn hình để tải mã nguồn về máy tính của bạn.</p>
      </div>
    </body>
    </html>
  `);
});

server.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
