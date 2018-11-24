  // /S/ac1.js
  //-----------------------

#img_file   = '#Sroot.ac1.jpg'
#map_file   = '#Sroot.ac1.txt'
#MAP_COUNT  = #map_count?10

  RC_[RC_.length] = {

#   title:   '#title',
#   name:    '#name#data_set',
    tablen:  RC_.length,
    imgid:   'img' + RC_.length,

    top: 0, width: 480, height: 360,
    imgtop: 0, imgwidth: 480, imgheight: (360 + 350),

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
            + 'top:' + (this.top + 142) + '; left: 135; width:100; font-size:80; '
            + 'color:#FBFBFB; text-align:center; user-select:none; display:none;"></div>';
      q.insertAdjacentHTML('afterend', r);
      this.reinit();
    },

    reinit: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      q.drawImage(RC_imgs[this.tablen], 0, this.imgtop, this.width, this.height, 0,  0, this.width, this.height);
      this.mode();
    },

    mode: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      q.drawImage(RC_imgs[this.tablen], 0, this.imgtop, this.width, this.height, 0,  0, this.width, this.height);
      if (this.power != 5) q.drawImage(RC_imgs[this.tablen], 407,  74,   8,  32, 399,  74,   8,  32);
      if (this.power != 6) q.drawImage(RC_imgs[this.tablen], 407, 125,   8,  32, 399, 126,   8,  32);
      if (this.power != 7) q.drawImage(RC_imgs[this.tablen], 407, 178,   8,  32, 399, 178,   8,  32);
      if (this.power != 8) q.drawImage(RC_imgs[this.tablen], 407, 230,   8,  32, 399, 230,   8,  32);
    },
