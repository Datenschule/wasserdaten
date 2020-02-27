const static char CONFIG_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head lang="en_US">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <title>OpenWaterData - Configuration</title>
    <style>
body{
  margin: 0;
  line-height: 1.45;
  font-family: sans-serif;
  background-color: #e2e8ed;
}
.wrapper {
  max-width: 600px;
  margin: auto;
}

header,
footer {
  padding: 1rem;
  background-color: #2fb5e9;
}
footer {
  padding-bottom: 2rem;
}

main {
  margin: 4rem 1rem 5rem;
}

label {
  display: block;
  margin-top: 2rem;
  font-weight: 600;
}

label input {
  display: block;
  padding: .5em .2em;
}

input[type=submit] {
  display: inline-block;
  margin-top: 2rem;
  padding: 1em;
  color: white;
  border: none;
  font: inherit;
  font-weight: 600;
  background-color: #fc704e;
  cursor: pointer;
}

input[type=submit]:hover,
input[type=submit]:active {
  background-color: #fc5c35;
}

input[type=submit]:disabled {
  background-color: grey;
  cursor: not-allowed;
}

    </style>
  </head>
  <body>
    <div class="wrapper">
      <header>OpenWaterData</header>
      <main>
        <h1>Configuration</h1>
        <p>Enter the name of the network (SSID) and the password below and hit <em>save</em>.</p>
        <p>The restart the board. It will try to log into the network you configured.</p>
        <form method="POST" action="/post_config">
          <label>
            SSID
            <input type="text" name="new_ssid" id="new_ssid" required>
          </label>
          <label>
            Password
            <input type="text" name="new_password" id="new_password" required>
          </label>
          <input type="submit" value="Save">
        </form>
      </main>
      <footer>
        <p>A project by<br>
        Open Knowledge Foundation Deutschland e.V.<br>
        Datenschule</p>
      </footer>
    </div>

    <script>
     async function postData(url = '', data = {}) {
       const response = await fetch(url, {
         method: 'POST', // *GET, POST, PUT, DELETE, etc.
         mode: 'cors', // no-cors, *cors, same-origin
         cache: 'no-cache', // *default, no-cache, reload, force-cache, only-if-cached
         credentials: 'same-origin', // include, *same-origin, omit
         headers: {
           'Content-Type': 'application/json'
           // 'Content-Type': 'application/x-www-form-urlencoded',
         },
         redirect: 'follow', // manual, *follow, error
         referrerPolicy: 'no-referrer', // no-referrer, *client
         body: JSON.stringify(data) // body data type must match Content-Type header
       });
       return await response.json(); // parses JSON response into native JavaScript objects
     }

     document.querySelector('form').addEventListener('submit', function(e) {
       e.preventDefault();
       document.querySelector('input[type=submit]').setAttribute("disabled", true);
       let new_ssid = document.querySelector('#new_ssid').value;
       let new_pw = document.querySelector('#new_password').value;

       postData('/post_config', { new_ssid: new_ssid, new_password: new_pw })
         .then((data) => {
           console.log(data); // JSON data parsed by `response.json()` call
         });
     })
    </script>
  </body>
</html>

)=====";
