#MAP_COUNT  = 10

<script type="text/javascript">
# var RC_debug = #debug?0;
# var RC_setup = #setup?0;
  var RC_      = new Array();
  var RC_imgs  = new Array();
  var img_i    = 0;
  RC_[RC_.length] = {

  // Title
  // --------------------------

#   title:   '#title?"Sample title"',
#   name:    '#name?"Sample_name"',
    tablen:  RC_.length,
    imgid:   'img' + RC_.length,

    top: 0, width: 480, height: 48,
    imgtop: 0, imgwidth: 480, imgheight: 96,

#   maps: #MAP_START,
#   mapc: #MAP_COUNT,

    cmap: [
        [ 0,  60,   2,  40,  40],
        [ 1,   2,   2,  60,  48], // 1) Back
        [ 2, 420,   2,  60,  48]  // 2) Next - none
    ],

#   pre_URL: '#pre_URL',
#   now_URL: '#now_URL',
#   nextURL: '#nextURL',

    finit: function () {
      var nn = this.tablen;
      var q = document.getElementById('view_area');
      var r  = '<div id="titl' + nn + '" style="position:absolute; top:' + (this.top + 4);
          r += '; left:100; width:330; height:30; font-size:28; color:#888888; font-weight:bold;';
          r += ' text-align:center; user-select:none;">' + this.title + '</div>';
      q.insertAdjacentHTML('afterend', r);
      this.reinit();
    },

    reinit: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      q.drawImage(RC_imgs[0], 0, this.imgtop, this.width, this.height, 0,  0, this.width, this.height);
      if (this.pre_URL != '') q.drawImage(RC_imgs[0],    0, 54, 42, 40,    0, 5, 42, 40);
      if (this.nextURL != '') q.drawImage(RC_imgs[0],  436, 54, 42, 40,  436, 5, 42, 40);
    },

    select: function (btn) {
      if ((btn == 1) || (btn == 2))
        location.href = ((btn == 1) ? this.pre_URL : this.nextURL);
    }
  };

  img_i = RC_imgs.length;
  RC_imgs[img_i] = new Image();
  RC_imgs[img_i].src = 
#<#Sroot.title.jpg:base64
