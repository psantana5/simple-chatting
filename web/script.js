window.addEventListener('load', function() {
    var chatBox = document.getElementById('chatBox');
    var messageInput = document.getElementById('messageInput');
    var sendButton = document.getElementById('sendButton');

    // Connect to the server
    var socket = new WebSocket('ws://localhost:8080');
    
    // Display received messages in the chat box
    socket.onmessage = function(event) {
        var message = event.data;
        var messageElement = document.createElement('p');
        messageElement.textContent = 'Server: ' + message;
        chatBox.appendChild(messageElement);
        chatBox.scrollTop = chatBox.scrollHeight;
    };
    
    // Send message when the send button is clicked
    sendButton.addEventListener('click', function() {
        var message = messageInput.value;
        if (message.trim() !== '') {
            socket.send(message);
            var messageElement = document.createElement('p');
            messageElement.textContent = 'Client: ' + message;
            chatBox.appendChild(messageElement);
            chatBox.scrollTop = chatBox.scrollHeight;
            messageInput.value = '';
        }
    });

    // Send message when Enter key is pressed in the message input field
    messageInput.addEventListener('keydown', function(event) {
        if (event.keyCode === 13) {
            event.preventDefault();
            sendButton.click();
        }
    });
});
