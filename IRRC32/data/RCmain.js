<script type="text/javascript">
// gurcmain.js
// (c) Goji 2017
//------------------------------------------------------------------------------
var xhr = new XMLHttpRequest();
xhr.onreadystatechange = HttpResponse;

function HttpResponse() {
  if (xhr.readyState == 4 && xhr.status == 200) {
    setStatus('#FFFFFF');
    if (xhr.responseText) {
      var vals = xhr.responseText.split('|');
      if (vals[0] == 'ERR') {
        alert(vals[1]);
      } else {
        document.forms.formset.mode.value = vals[1];
        document.forms.formset.val0.value = vals[3];
        document.forms.formset.val1.value = ((vals.length > 4) ? vals[4] : '');
      }
    }
  }
}

function HttpReqSend(val) {
  xhr.open('GET', '/IR'+val);
  xhr.send(null);
}

//------------------------------------------------------------------------------
var edit_mode = 'N', edit_tbln = -1;
var draw_tbln = -1, xx = 0, yy = 0, ww = 0, hh = 0, pickup = 0;

document.getElementById('view_area').onclick = function (event) {
  var x = parseInt(event.clientX + window.pageXOffset - 8);
  var y = parseInt(event.clientY + window.pageYOffset);
  var res = 'x=' + x + ',y=' + y;
  var i = 0;
  if (edit_mode == 'Y') {
    i = edit_tbln;
  } else {
    for (i = 0; i < (RC_.length - 1); i++)
      if ((y >= RC_[i].top) && (y <= RC_[i + 1].top)) break;
    if (i >= (RC_.length - 1)) return;
  }
  if ((y >= RC_[i].top) && (y <= RC_[i + 1].top)) {
    y -= RC_[i].top;
    res = 'x=' + x + ',y=' + y;
    var p = RC_[i].cmap;
    for (var j = 0; j < p.length; j++) {
      if ((x >= p[j][1]) && (x <= (p[j][1] + p[j][3])) && (y >= p[j][2]) && (y <= (p[j][2] + p[j][4]))) {
        if (j == 0) {
          res += ',BtnEdit(' + i + ')';
          if (RC_setup) BtnEdit(i);
        } else {
          var cp = RC_[i].maps + p[j][0];
          res += ',BtnClick(' + cp + ')';
          BtnClick(i, j);
        }
      }
    }
  }
  if (RC_debug) document.getElementById((RC_debug > 1) ? ('cpos' + i) : 'cpos0').textContent = res;
}


//------------------------------------------------------------------------------

document.getElementById('view_area').onmousedown = function (event) {
  if (pickup == 1) {
    pickup++;
    draw_tbln = edit_tbln;
    var p = RC_[draw_tbln];
    xx = parseInt(event.clientX + window.pageXOffset - 8);
    yy = parseInt(event.clientY + window.pageYOffset - p.top);
    document.forms.formset.x.value = xx;
    document.forms.formset.y.value = yy;
    document.forms.formset.w.value = 0;
    document.forms.formset.h.value = 0;
  }
}

document.getElementById('view_area').onmousemove = function (event) {
  if (pickup == 2) {
    var p = RC_[draw_tbln];
    var x = parseInt(event.clientX + window.pageXOffset - 8);
    var y = parseInt(event.clientY + window.pageYOffset - p.top);
    if ((x > xx) && (y > yy)) {
      document.forms.formset.w.value = w = (x - xx);
      document.forms.formset.h.value = h = (y - yy);
      clickCheck(1);
      document.forms.formset.uMAP.value = 'U';
    }
  }
}

document.getElementById('view_area').onmouseup = function (event) {
 if (pickup == 2) pickup = 0;
}

//------------------------------------------------------------------------------
function BtnEdit(val) {
  if (val > 1000) {
    var btn = Number(document.forms.formset.btn.value);
    if (isNaN(btn) || (btn == 0) || (btn > RC_[edit_tbln].mapc)) {
      document.forms.formset.btn.value = '';
      return;
    }
    if (val == 9000) {  // posXY
      pickup = (pickup != 1) ? 1 : 0;
      return;
    }
    setStatus('#808080');
    if ((val == 9001) || (val == 9002)) { // 9001:Read, 9002:Load
      document.forms.formset.val0.value = '';
      document.forms.formset.val1.value = '';
      if (val == 9001)
        HttpReqSend('?exe=Read&btn=' + unitBtn(edit_tbln, btn));
      else
        BtnClick(edit_tbln, btn);
    } else if (val == 9003) {             // 9003:Save
      HttpReqSend('?exe=Save&btn=' + unitBtn(edit_tbln, btn)
        + '&md=' + ((document.forms.formset.mode.value != 'NEC') ? 'EHA' : 'NEC')
        + '&v0=' + document.forms.formset.val0.value
        + '&v1=' + document.forms.formset.val1.value);
    }
    if ((val == 9001) || (val == 9003)) document.forms.formset.uDAT.value = 'U';
    return;
  }
  if (edit_mode != 'Y') {
    edit_mode = 'Y';
    edit_tbln = val;
    document.forms.formset.btn.value  = '';
    document.forms.formset.mode.value = '';
    document.forms.formset.val0.value = '';
    document.forms.formset.val1.value = '';
    document.forms.formset.x.value    = '';
    document.forms.formset.y.value    = '';
    document.forms.formset.w.value    = '';
    document.forms.formset.h.value    = '';
    document.forms.formset.tabs.value = '(' + RC_[val].maps + ')';
    document.forms.formset.uMAP.value = '';
    document.forms.formset.uDAT.value = '';
    document.getElementById('valset').style.left = 20;
    document.getElementById('valset').style.top  = 10 + RC_[val + 1].top;
    document.getElementById('valset').style.display = 'block';
  } else {
    document.getElementById('valset').style.display = 'none';
    clickCheck(0);
    if (document.forms.formset.uMAP.value == 'U') {
      if (confirm('update MAP file : "' + RC_[edit_tbln].mapfile + '" ?')) {
        var reqs = '?exe=Update&v0=' + RC_[edit_tbln].mapfile + '|';
        var xll = (RC_[edit_tbln].mapc > RC_[edit_tbln].cmap.length) ? RC_[edit_tbln].cmap.length : RC_[edit_tbln].mapc;
        for (var xl = 0; (xl < xll); xl++) {
          reqs += '[';
          var xcl = RC_[edit_tbln].cmap[xl].length;
          for (var xc = 0; (xc < xcl); xc ++) {
            reqs += num2dec4(RC_[edit_tbln].cmap[xl][xc]);
            if ((xc + 1) >= xcl) break;
            reqs += ',';
          }
          reqs += ']';
          if ((xl + 1) >= xll) break;
          reqs += ',|';
        }
        reqs += '|';
        HttpReqSend(reqs);
      }
      document.forms.formset.uMAP.value == '';
    }
    if (document.forms.formset.uDAT.value == 'U') {
      if (confirm('update DEF file : "' + RC_[edit_tbln].deffile + '" ?'))
        HttpReqSend('?exe=Update&v1='
          + RC_[edit_tbln].deffile + '|'
          + RC_[edit_tbln].maps + '|'
          + (RC_[edit_tbln].maps + RC_[edit_tbln].mapc) + '|');
      document.forms.formset.uDAT.value == '';
    }
    edit_mode = 'N';
  }
}

function BtnClick(n, btn) {
  var req;
  if (edit_mode == 'Y') {
    document.forms.formset.btn.value = btn;
    if (RC_[n].cmap[btn].length < 5)
      RC_[n].cmap[btn] = [ 0, 100, 100, 100, 100 ];
    document.forms.formset.x.value   = RC_[n].cmap[btn][1];
    document.forms.formset.y.value   = RC_[n].cmap[btn][2];
    document.forms.formset.w.value   = RC_[n].cmap[btn][3];
    document.forms.formset.h.value   = RC_[n].cmap[btn][4];
    clickCheck(1);
    setStatus('#808080');
    if (req = unitBtn(n, btn)) HttpReqSend('?exe=Load&btn=' + req);
  } else {
    var req = RC_[n].select(btn);
    if ((req != '') && (req != 'undefined')) HttpReqSend('?exe=Send&btn=' + req);
  }
}

function unitBtn(n, btn) {
  if (!!RC_[n].unitcnt) {
    if (RC_[n].unitsel == 0) return(0);
    for (var i = RC_[n].unitmask, j = 0; (i); (i >>= 1), j++)
      if (RC_[n].unitsel & i) return(RC_[n].maps + btn + (RC_[n].unitmul * j));
    return(0);
  }
  return(RC_[n].maps + btn);
}

function setStatus(bc) {
  document.forms.formset.mode.style.backgroundColor = bc;
  document.forms.formset.val0.style.backgroundColor = bc;
  document.forms.formset.val1.style.backgroundColor = bc;
}

//------------------------------------------------------------------------------
function clickCheck(sw) {
  if (draw_tbln != -1) RC_[draw_tbln].reinit();
  draw_tbln = -1;
  if (sw == 1) {
    draw_tbln = edit_tbln;
    xx   = parseInt(document.forms.formset.x.value);
    yy   = parseInt(document.forms.formset.y.value);
    ww   = parseInt(document.forms.formset.w.value);
    hh   = parseInt(document.forms.formset.h.value);
    var p = RC_[draw_tbln];
    var views = document.getElementById(p.imgid).getContext('2d');
    if ((xx * yy * ww * hh) != 0) {
      var btn = Number(document.forms.formset.btn.value);
      views.lineWidth = 5;
      views.setLineDash([3, 3]);
      views.strokeStyle = '#FFFFFF';
      views.beginPath();
      views.strokeRect(xx, yy, ww, hh);
      if ((p.cmap[btn][0] != btn)||
          (p.cmap[btn][1] != xx) ||(p.cmap[btn][2] != yy) ||
          (p.cmap[btn][3] != ww) ||(p.cmap[btn][4] != hh)) {
        p.cmap[btn][1] = xx;
        p.cmap[btn][2] = yy;
        p.cmap[btn][3] = ww;
        p.cmap[btn][4] = hh;
        document.forms.formset.uMAP.value = 'U';
      }
    } else draw_tbln = -1;
  }
}

//------------------------------------------------------------------------------
function view_rc() {
  var addelm = document.getElementById('view_area');
  var canvas = '';
  for (var i = 0; i < (RC_.length); i++) {
    if (i == 0) {
      RC_[i].top   = 0;
      RC_[i].maps  = 0;
    } else {
      RC_[i].top   = (RC_[i - 1].top  + RC_[i - 1].height);
      if (RC_[i].maps == 0) RC_[i].maps += (RC_[i - 1].maps + ((i > 1) ? RC_[i - 1].mapc : 0));
    }
    canvas += '<canvas id=' + RC_[i].imgid + ' width=' + RC_[i].width + ' height=' + RC_[i].height
           +  ' style="position:absolute; top:' + RC_[i].top + '"></canvas>';
    if (RC_debug) {
      if ((RC_debug > 1) || (i == 0))
        canvas += '<div id="cpos' + i + '" style="position:absolute; top:' + (RC_[i].top + 14)
               +  '; left:55; font-size:12; color:#FFFFFF;"></div>';
    }
  }
  addelm.insertAdjacentHTML('afterbegin', canvas);
  for (var i = 0; i < (RC_.length - 1); i++) RC_[i].finit();

}

function num2hex2(n) { return((  '0' + n.toString(16)).slice(-2)); }
function num2dec2(n) { return((  ' ' + n.toString(10)).slice(-2)); }
function num2dec3(n) { return(( '  ' + n.toString(10)).slice(-3)); }
function num2dec4(n) { return(('   ' + n.toString(10)).slice(-4)); }
</script>
