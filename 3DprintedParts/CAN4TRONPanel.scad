$fn = 30;

widthPx = 64;
heightPx = 32;
gridPitchMm = 4;
textPitchMm = 2.5;

panelDepth = 14;
gridPanelWidth = widthPx*gridPitchMm;
gridPanelHeight = heightPx*gridPitchMm;
gridPanelInset = min(gridPanelWidth, gridPanelHeight) * 3/16;
textPanelWidth = widthPx*textPitchMm;
textPanelHeight = heightPx*textPitchMm;
textPanelInset = min(textPanelWidth, textPanelHeight) * 5/16;

clearance = 0.4;
frameWall = 2;
frameDepth = panelDepth + frameWall*2;

gridPanelOuterHeight = (gridPanelHeight + frameWall)*2;
textPanelOuterHeight = textPanelHeight + frameWall*2;

module duoY(dx, dy) {
    translate([dx, +dy]) rotate([0,0,180]) children();
    translate([dx, -dy]) children();
}

module duoYX(dx, dy) {
    translate([+dx, -dy]) children();
    translate([-dx, +dy]) children();
}

module duoX(dx, dy) {
    translate([+dx, dy]) children();
    translate([-dx, dy]) children();
}

module quad(dx, dy) {
    duoX(dx, 0) duoY(0, dy) children();
}

module gridPanel() {
    linear_extrude(height=frameDepth, center=true) difference() {
        square([gridPanelWidth + 2*frameWall, gridPanelHeight + 2*frameWall], center=true);
        duoX(gridPanelWidth/4-gridPanelInset/8,0) square([gridPanelWidth/2-gridPanelInset*3/4, gridPanelHeight-gridPanelInset], center=true);
        duoX(gridPanelWidth/4,0) square([gridPanelWidth*7/16, gridPanelHeight/4], center=true);
        duoY(0, 55.5) circle(1.5+clearance);
        duoX(120, 24.3) circle(1.4+clearance);
        quad(120, 55.5) circle(1.5+clearance);
    }
}

module textPanel() {
    linear_extrude(height=frameDepth, center=true) difference() {
        square([textPanelWidth + 2*frameWall, textPanelHeight + 2*frameWall], center=true);
        square([textPanelWidth-textPanelInset+4, textPanelHeight-textPanelInset], center=true);
        duoYX(12.5, 65/2) circle(1.5+clearance);
        duoYX(72.5, 15.5) circle(1.4+clearance);
        quad(62.5, 65/2) circle(1.5+clearance);
    }
}

module logo() {
    holeDiameter=2;
    holeDepth=16.5;
    difference() {
        linear_extrude(height=panelDepth+frameWall*2, center=true) scale([1.35,1.3,1]) import("CAN4tron.svg", center=true);
        translate([-120,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([-78.5,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([-56.5,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([-33,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([-24,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([-9,-10,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([13,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([42,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([71,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([98,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
        translate([124.2,0,-panelDepth/2-frameWall+holeDepth/2]) cylinder(h=holeDepth,r=holeDiameter/2,center=true);
    }
}

module oneBigPiece() {
    union() {
        translate([0,gridPanelOuterHeight/2,0]) difference() {
            duoY(0, gridPanelHeight/2) gridPanel();
            translate([0, 0, (frameDepth-panelDepth)/2]) linear_extrude(height=panelDepth, center=true)
                square([gridPanelWidth + clearance*2, gridPanelHeight*2 + clearance*2], center=true);
        }
        translate([0,-textPanelOuterHeight/2,0]) difference() {
            duoX(textPanelWidth/2, 0) textPanel();
            translate([0, 0, (frameDepth-panelDepth)/2]) linear_extrude(height=panelDepth, center=true)
                square([textPanelWidth*2 + clearance*2, textPanelHeight + clearance*2], center=true);
        }
        translate([0,gridPanelOuterHeight+19,0]) logo();
    }
}

module innerBottomPlate() {
    intersection() {
        oneBigPiece();
        translate([0,gridPanelOuterHeight-frameWall-280/2,frameWall])
            cube([gridPanelWidth+clearance*2, 280+clearance*2, frameDepth], center=true);
    }
}

module panelLowerLeft() {
    intersection() {
        difference() {
            oneBigPiece();
            translate([0,gridPanelOuterHeight-frameWall-280/2,frameWall])
                cube([gridPanelWidth+clearance*2, 280+clearance*2, frameDepth], center=true);
        }
        translate([-(textPanelWidth+frameWall)/2, 45, 0]) cube([textPanelWidth+frameWall, 280, frameDepth], center=true);
    }
}

module panelLowerRight() {
    intersection() {
        difference() {
            oneBigPiece();
            translate([0,gridPanelOuterHeight-frameWall-280/2,frameWall])
                cube([gridPanelWidth+clearance*2, 280+clearance*2, frameDepth], center=true);
        }
        translate([(textPanelWidth+frameWall)/2, 45, 0]) cube([textPanelWidth+frameWall, 280, frameDepth], center=true);
    }
}

module panelTopLogo() {
    intersection() {
        difference() {
            oneBigPiece();
            translate([0,gridPanelOuterHeight-frameWall-280/2,frameWall])
                cube([gridPanelWidth+clearance*2, 280+clearance*2, frameDepth], center=true);
            translate([0,gridPanelOuterHeight+20,frameWall*3])
                cube([gridPanelWidth+frameWall*2, 40, frameDepth], center=true);
        }
        translate([0, 280+45, 0]) cube([280, 280, frameDepth], center=true);
    }
}

module panelTopLogoCAN() {
    intersection() {
        oneBigPiece();
        translate([-79.35-clearance/4,gridPanelOuterHeight+20+clearance/4,frameWall*3])
            cube([102, 40, frameDepth], center=true);
    }
}

module panelTopLogo4() {
    intersection() {
        oneBigPiece();
        union() {
            translate([-23.4+clearance/4,gridPanelOuterHeight+20+clearance/4,frameWall*3])
                cube([10, 40, frameDepth], center=true);
            translate([-5.35-clearance/4,gridPanelOuterHeight+12.5+clearance/4,frameWall*3])
                cube([26, 25, frameDepth], center=true);
        }
    }
}

module panelTopLogoTRON() {
    intersection() {
        oneBigPiece();
        translate([68.59+clearance/4,gridPanelOuterHeight+20+clearance/4,frameWall*3]) {
            cube([122, 40, frameDepth], center=true);
            translate([-70,14.3,0]) cube([18, 11, frameDepth], center=true);
        }
    }
}

module stand() {
    standHeight=230;
    standDepth=170;
    standWidth=50;
    standSmallerWidth=12;
    difference() { union() {
        translate([0,standHeight/2-textPanelOuterHeight,-frameDepth/2-1.5]) linear_extrude(height=3, center=true) difference() {
            square([standWidth, standHeight], center=true);
            polygon([[standSmallerWidth/2,(standHeight-textPanelOuterHeight)/2],[standSmallerWidth/2,standHeight/2],
                [standWidth/2,standHeight/2],[standWidth/2,textPanelOuterHeight-standHeight/2]]);
            polygon([[-standSmallerWidth/2,(standHeight-textPanelOuterHeight)/2],[-standSmallerWidth/2,standHeight/2],
                [-standWidth/2,standHeight/2],[-standWidth/2,textPanelOuterHeight-standHeight/2]]);
            translate([0,(textPanelOuterHeight-standHeight)/2]) duoY(textPanelWidth/2 - 62.5, 65/2) circle(1.5+clearance);
            translate([0,(textPanelOuterHeight-standHeight)/2]) duoY(62.5 - textPanelWidth/2, 65/2) circle(1.5+clearance);
            translate([0,textPanelOuterHeight+(gridPanelOuterHeight-standHeight-gridPanelHeight)/2]) duoY(0, 55.5) circle(1.5+clearance);
            translate([0,textPanelOuterHeight+(gridPanelOuterHeight-standHeight+gridPanelHeight)/2-55.5]) circle(1.5+clearance);
        }
        translate([0,0,-6]) rotate([270,45,0]) linear_extrude(height=2.5, center=true) difference() {
            polygon([[15,-10],[-10,15],[27,standDepth*0.65],[standDepth*0.65,27]]);
            translate([ 4, 4]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([ 7,17]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([17, 7]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([20,20]) scale(2.25) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([30, 10]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([10,30]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([45,25]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([25,45]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([43,13]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([13,43]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([47,37]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([37,47]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([16,56]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([56,16]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([28,58]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([58,28]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([19,69]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([69,19]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([49,49]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([71,31]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([31,71]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([60,40]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([40,60]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([82,22]) polygon([[0,0],[10,2],[12,12],[2,10]]);
            translate([22,82]) polygon([[0,0],[10,2],[12,12],[2,10]]);
        }
        translate([4.9,-textPanelOuterHeight,-frameDepth/2-2]) rotate([0,60,0]) linear_extrude(height=2.5, center=true) difference() {
            polygon([[0,0],[0,standHeight],[standDepth,0]]);
            polygon([[10,0],[10,13],[40,13]]);
            polygon([[30,30],[10,30],[10,77],[standDepth/2.3,77],[standDepth/1.7,55]]);
            polygon([[10,100],[10,140],[standDepth/5,140],[standDepth/2.3,100]]);
        }
        translate([-4.9,-textPanelOuterHeight,-frameDepth/2-2]) rotate([0,120,0]) linear_extrude(height=2.5, center=true) difference() {
            polygon([[0,0],[0,standHeight],[standDepth,0]]);
            polygon([[10,0],[10,13],[40,13]]);
            polygon([[30,30],[10,30],[10,77],[standDepth/2.3,77],[standDepth/1.7,55]]);
            polygon([[10,100],[10,140],[standDepth/5,140],[standDepth/2.3,100]]);
        }}
        translate([0,-textPanelOuterHeight,-frameDepth/2-standDepth/2]) rotate([22,0,0]) cube([standDepth*2,63.6,standDepth],center=true);
    }
}

//render() innerBottomPlate();
//render() panelLowerLeft();
//render() panelLowerRight();
//render() panelTopLogo();
//render() panelTopLogoCAN();
//render() panelTopLogo4();
//render() panelTopLogoTRON();
//render() logo();
render() stand();
render() oneBigPiece();
