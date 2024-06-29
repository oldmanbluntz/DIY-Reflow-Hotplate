document.getElementById('uploadForm').onsubmit = function() {
  setTimeout(function() {
    window.location.href = '/';
  }, 2000); // Adjust the delay as needed
};

document.getElementById('startForm').onsubmit = function() {
  setTimeout(function() {
    window.location.href = '/';
  }, 1000); // Adjust the delay as needed
};

document.getElementById('stopForm').onsubmit = function() {
  setTimeout(function() {
    window.location.href = '/';
  }, 1000); // Adjust the delay as needed
};

function updateLEDStatus() {
  fetch('/ledstatus')
    .then(response => response.json())
    .then(data => {
      document.getElementById('ledWhite').textContent = data.ledWhite ? 'ON' : 'OFF';
      document.getElementById('ledGreen').textContent = data.ledGreen ? 'ON' : 'OFF';
      document.getElementById('ledOrange').textContent = data.ledOrange ? 'ON' : 'OFF';
      document.getElementById('ledRed').textContent = data.ledRed ? 'ON' : 'OFF';
    })
    .catch(error => console.error('Error:', error));
}

setInterval(updateLEDStatus, 1000); // Update LED status every second
