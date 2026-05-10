use <polyround.scad>;

$fn = 40;
mil=2.54;
clearance=0.4;

module board(w,h,cl,mountingHoles) {
  hh=2.5;
  ro=4;
  rh=1;
  radiiPoints=[[0,0,ro],[0,(h/2-7.5)*mil+2*cl,rh],[mil,(h/2-7.5)*mil+2*cl,rh],[mil,(h/2-3.5)*mil,rh],[0,(h/2-3.5)*mil,rh],
      [0,h*mil+2*cl,ro],[(w/2-5)*mil+2*cl,h*mil+2*cl,rh],[(w/2-5)*mil+2*cl,(h-1)*mil+2*cl,rh],[(w/2+5)*mil,(h-1)*mil+2*cl,rh],[(w/2+5)*mil,h*mil+2*cl,rh],
      [w*mil+2*cl,h*mil+2*cl,ro],[w*mil+2*cl,(h/2+7.5)*mil,rh],[(w-1)*mil+2*cl,(h/2+7.5)*mil,rh],[(w-1)*mil+2*cl,(h/2+3.5)*mil+2*cl,rh],[w*mil+2*cl,(h/2+3.5)*mil+2*cl,rh],
      [w*mil+2*cl,0,ro],[(w/2+5)*mil,0,rh],[(w/2+5)*mil,mil,rh],[(w/2-5)*mil+2*cl,mil,rh],[(w/2-5)*mil+2*cl,0,rh]];
  difference() {
    translate([0,0,1.99]) cube([w*mil+4+2*cl,h*mil+4+2*cl,8],center=true);
    difference() {
      translate([-w*mil/2-cl,-h*mil/2-cl,0]) polyRoundExtrude(radiiPoints, 6, 0, 2.5, 10);
      for(mh = mountingHoles) {
        translate([mh[0]*mil, mh[1]*mil,hh/2]) cylinder(h=hh+0.01,r=3+cl/2, center=true);
      }
    }
    for(mh = mountingHoles) {
      translate([mh[0]*mil, mh[1]*mil, hh/2-1]) cylinder(h=hh+4, r=1.75+cl/2, center=true);
    }
    
    translate([-w*mil/2-1-cl,18.5*mil,2+hh+1.6]) cube([2.1,2*mil+2*cl,4],center=true);
    translate([w*mil/2+1+cl,18.5*mil,2+hh+1.6]) cube([2.1,2*mil+2*cl,4],center=true);
    
    translate([-w*mil/2-1-cl,0,2+hh+1.6]) cube([2.1,4*mil+2*cl,4],center=true);
    translate([w*mil/2+1+cl,0,2+hh+1.6]) cube([2.1,4*mil+2*cl,4],center=true);
    
    translate([-w*mil/2-1-cl,-18.5*mil,2+hh+1.6]) cube([2.1,2*mil+2*cl,4],center=true);
    translate([w*mil/2+1+cl,-18.5*mil,2+hh+1.6]) cube([2.1,2*mil+2*cl,4],center=true);
  }
}

module baseBoardV1_0() {
  board(32,42,clearance,[[-11,15],[13,15],[-0.5,6.5],[0.5,-6.5],[-13,-15],[11,-15]]);
}

module commanderBoardV1_0() {
  board(25,42,clearance,[[9.5,7],[9.5,-7],[-11.5,11],[-11.5,-11],[3,16.5],[3,-16.5]]);
}

difference() {
  union() {
    translate([-32*mil-4-2*clearance,0,0]) baseBoardV1_0();
    baseBoardV1_0();
    translate([+28.5*mil+4+2*clearance,0,0]) commanderBoardV1_0();
  }
  //translate([0,-50,0]) cube([280,100,20],center=true);
}
