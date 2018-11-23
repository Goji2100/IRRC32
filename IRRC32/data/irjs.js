<script type="text/javascript">
var ex_mode = 'S', bcON = '#F0F8FF', bcOFF = '#CCCCCC';
var ac_power = 0, ac_mode = 2, ac_temp = 25, ac_base = 300;
var ac_modec = new Array(' ', 'H', 'D', 'C');

var xhr = new XMLHttpRequest();
xhr.onreadystatechange = HttpResponse;

function HttpResponse() {
  if (xhr.readyState == 4 && xhr.status == 200)
    if (xhr.responseText) {
      var vals = xhr.responseText.split('|');
      if (vals[0] == 'ERR') alert(vals[1]);
    }
}

function HttpReqSend(val) {
  xhr.open('GET', '/IR' + val);
  xhr.send(null);
}

function setCmnd(mode) {
  return ('?exe=' + (mode == 'R' ? 'Read' : 'Send') + '&btn=');
}

function setACbtn() {
  return (0 + ac_base + (ac_mode * 15) + (ac_temp - 20));
}

function setStatus(id, bc) {
  document.getElementById(id).style.backgroundColor = bc;
}

function Btn(btn) {
  var udev = parseInt(btn / 100); // CL=1, TV=2, AC=3
  var btnn = (btn % 100);
  if (udev == 3) {   // AirCON ?
    if (btnn == 0) { // Power ?
      if (ac_power == 0) {
        ac_power = 1;
        HttpReqSend(setCmnd(ex_mode) + setACbtn());
      } else {
        ac_power = 0;
        HttpReqSend(setCmnd(ex_mode) + ac_base);
      }
      setStatus('acm_P', (ac_power == 0 ? bcOFF : bcON));
    } else if ((btnn >= 1) && (btnn <= 3)) {
      if (ac_mode != 0) setStatus('acm_' + ac_modec[ac_mode], bcOFF);
      ac_mode = btnn;
      setStatus('acm_' + ac_modec[ac_mode], bcON);
      if (ac_power == 1) HttpReqSend(setCmnd(ex_mode) + setACbtn());
    } else if ((btnn == 4) || (btnn == 5)) {
      if ((btnn == 4) && (ac_temp > 20)) ac_temp -= 1;
      if ((btnn == 5) && (ac_temp < 30)) ac_temp += 1;
      document.getElementById('ac_temp').value = ac_temp;
      if (ac_power != 0) HttpReqSend(setCmnd(ex_mode) + setACbtn());
    }
    if (btnn < 90) return;
  }
  if ((btnn == 98) || (btnn == 99)) {
    setStatus('ex_R', bcOFF);
    setStatus('ex_S', bcOFF);
    ex_mode = (btnn == 98 ? 'R' : 'S');
    setStatus('ex_' + ex_mode, bcON);
  } else HttpReqSend(setCmnd(ex_mode) + btn);
}
</script>
