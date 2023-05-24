let macAddress; // Declare global variable for MAC address

// Function to fetch and display the current credentials
function getCredentials() {
  fetch("/credentials")
    .then((response) => response.text()) // Read the response as plain text
    .then((data) => {
      const [ssid, password, fetchedMacAddress, shortCode] = data.split("\n"); // Split the response by newlines
      document.getElementById("ssidValue").innerText = ssid.replace(
        "SSID: ",
        ""
      ); // Update the SSID element
      document.getElementById("passwordValue").innerText = password.replace(
        "Password: ",
        ""
      ); // Update the password element
      document.getElementById("macAddressValue").innerText =
        fetchedMacAddress.replace("MAC Address: ", ""); // Update the MAC address element
      document.getElementById("shortCodeValue").innerText = shortCode.replace(
        "Short Code: ",
        ""
      ); // Update the Short Code element
      macAddress = fetchedMacAddress.replace("MAC Address: ", ""); // Assign the fetched MAC address to the global variable
    })
    .catch((error) => console.error("Error:", error));
}

// Call the function to fetch and display credentials on page load
window.addEventListener("DOMContentLoaded", getCredentials);
