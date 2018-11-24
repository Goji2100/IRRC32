  // /S/ac2.js
  //-----------------------

#img_file   = '#Sroot.ac2.jpg'
#map_file   = '#Sroot.ac2.txt'
#MAP_COUNT  = #map_count?10

  RC_[RC_.length] = {

#   title:   '#title',
#   name:    '#name#data_set',
    tablen:  RC_.length,
    imgid:   'img' + RC_.length,

    top: 0, width: 480, height: 360,
    imgtop: 0, imgwidth: 480, imgheight: (360 + 420),

#   mapfile: '#map_file',
#   maps: #MAP_START,
#   mapc: #MAP_COUNT,

    cmap: [
#<#map_file
    ],

    finit: function () {
      var nn = this.tablen;
      var q = document.getElementById('view_area');
      var r = '<div id="titl' + nn + '" style="position:absolute; top:' + (this.top + 4) + '; '
            + 'left:100; width:340; height:30; font-size:28; color:#FBFBFB; font-weight:bold; '
            + 'text-align:right; user-select:none;">' + this.title + '</div>'
            + '<div id="temp' + nn + '" style="position:absolute; '
            + 'top:' + (this.top + 142) + '; left: 190; width:100; font-size:80; '
            + 'color:#FBFBFB; text-align:center; user-select:none; display:none;"></div>';
      q.insertAdjacentHTML('afterend', r);
      this.reinit();
    },

    reinit: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      q.drawImage(RC_imgs[this.tablen], 0, this.imgtop, this.width, this.height, 0,  0, this.width, this.height);
      this.mode();
      this.fans();
      this.louvs();
    },

    mode: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      q.drawImage(RC_imgs[this.tablen], 300, this.imgtop, this.width - 300, this.height, 300,  0, this.width - 300, this.height);
      if (this.power != 5) q.drawImage(RC_imgs[this.tablen], 440,  74,   8,  32, 432,  74,   8,  32);
      if (this.power != 6) q.drawImage(RC_imgs[this.tablen], 440, 125,   8,  32, 432, 126,   8,  32);
      if (this.power != 7) q.drawImage(RC_imgs[this.tablen], 440, 178,   8,  32, 432, 178,   8,  32);
      if (this.power != 8) q.drawImage(RC_imgs[this.tablen], 440, 230,   8,  32, 432, 230,   8,  32);
    },

    fans: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      var xx = parseInt(this.fan.val % 4) * 104, yy = parseInt(this.fan.val / 4) * 104;
      q.drawImage(RC_imgs[this.tablen], xx + 1, this.imgtop + 362 + yy, 103, 103,  17,  70, 103, 103);
    },

    louvs: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      var xx = parseInt(this.louv.val % 4) * 104, yy = parseInt(this.louv.val / 4) * 104;
      q.drawImage(RC_imgs[this.tablen], xx + 1, this.imgtop + 570 + yy, 103, 103,  17, 223, 103, 103);
    },
