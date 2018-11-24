  // xxSTD.js
  // --------------------------

#   deffile: '#def_file',
#<#def_file:irdata:#MAP_START:#MAP_COUNT

    power: 0,

#unit_count = #unit_count?1

#tleft      = 100
#twidth     = 340

#if #unit_count == 2
    unitcnt: 2, unitmask: 4, unitsel: 4, unitmul: 10,
#tleft      = 160
#twidth     = 280
#endif

#if #unit_count == 3
    unitcnt: 3, unitmask: 4, unitsel: 4, unitmul: 10,
#tleft      = 210
#twidth     = 230
#endif

    finit: function () {
      var nn = this.tablen;
      var q = document.getElementById('view_area');
      var r  = '<div id="titl' + nn + '" style="position:absolute; top:' + (this.top + 4);
#         r += '; left:#tleft; width:#twidth; height:30; font-size:28; color:#FBFBFB; font-weight:bold;';
          r += ' text-align:right; user-select:none;">' + this.title + '</div>';
      q.insertAdjacentHTML('afterend', r);
      this.reinit();
    },

#if #unit_count == 1
    reinit: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      q.drawImage(RC_imgs[this.tablen], 0, this.imgtop, this.width, this.height, 0,  0, this.width, this.height);
    },

    select: function (btn) {
      return(this.maps + this.cmap[btn][0]);
    }
#else
    reinit: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
      q.drawImage(RC_imgs[this.tablen], 0, this.imgtop, this.width, this.height, 0,  0, this.width, this.height);
      this.setdevsel();
    },

    setdevsel: function () {
      var q = document.getElementById(this.imgid).getContext('2d');
                             q.drawImage(RC_imgs[this.tablen],  40, this.imgtop + 250, 36, 36,  60,  8, 36, 36);
                             q.drawImage(RC_imgs[this.tablen], 120, this.imgtop + 250, 36, 36, 110,  8, 36, 36);
#if #unit_count == 3
                             q.drawImage(RC_imgs[this.tablen], 200, this.imgtop + 250, 36, 36, 160,  8, 36, 36);
#endif
      if (this.unitsel &  4) q.drawImage(RC_imgs[this.tablen],  80, this.imgtop + 250, 36, 36,  60,  8, 36, 36);
      if (this.unitsel &  2) q.drawImage(RC_imgs[this.tablen], 160, this.imgtop + 250, 36, 36, 110,  8, 36, 36);
      if (this.unitsel &  1) q.drawImage(RC_imgs[this.tablen], 240, this.imgtop + 250, 36, 36, 160,  8, 36, 36);
    },

    select: function (btn) {
      if ((btn >= 7) && (btn <= 9)) {
        if (btn == 7) this.unitsel ^= 4;
        if (btn == 8) this.unitsel ^= 2;
        if (btn == 9) this.unitsel ^= 1;
        this.setdevsel();
        return('');
      }
      var btnn = this.cmap[btn][0];
      var rets = '';
      if (this.unitsel & 4) rets += this.maps + (btnn                      ) + '|';
      if (this.unitsel & 2) rets += this.maps + (btnn  + (this.unitmul    )) + '|';
      if (this.unitsel & 1) rets += this.maps + (btnn  + (this.unitmul * 2)) + '|';
      return(rets);
    }
#endif
  };

  img_i = RC_imgs.length;
  RC_imgs[img_i] = new Image();
  RC_imgs[img_i].src = 
#<#img_file:base64
