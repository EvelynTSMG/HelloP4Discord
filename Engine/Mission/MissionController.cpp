#include "MissionController.h"
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <string>
#include "Units/Spear.h"
#include "Units/Projectile.h"
#include "../Math/PVector.h"
#include "Units/HitboxFrame.h"
#include "../V4Core.h"

///move to Func::numDigits later
template <class T>
int numDigits(T number) //stolen from stackoverflow
{
    int digits = 0;
    if (number < 0) digits = 1;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

MissionController::MissionController()
{
    ///first initialization, fill the buffers

    patapon = new Patapon;
    kacheek = new Kacheek;
    kacheek2 = new Kacheek;
    kacheek3 = new Kacheek;
    endFlag1 = new EndFlag;
    feverworm = new FeverWorm;
    hatapon = new Hatapon;

}

void MissionController::addDmgCounter(int type, int damage, float baseX, float baseY)
{
    DamageCounter tmp;
    tmp.type = type;
    tmp.damage = damage;

    cout << "MissionController::addDmgCounter(" << type << ", " << damage << ", " << baseX << ", " << baseY << ")" << endl;

    int digits = numDigits(damage);
    string sdigits = to_string(damage);

    cout << "Digits: " << digits << " " << sdigits << endl;

    int separator = 0;
    float init_scale = 1;
    float dg_scale = 4;

    switch(type)
    {
        case 0:
        {
            separator = 12;
            init_scale = 0.8;
            dg_scale = 2.8;
            break;
        }

        case 1:
        {
            separator = 16;
            init_scale = 0.8;
            dg_scale = 3.5;
            break;
        }

        case 2:
        {
            separator = 20;
            init_scale = 0.8;
            dg_scale = 4.1;
            break;
        }
    }

    for(int i=0; i<digits; i++)
    {
        string sdigit = string()+sdigits[i];
        int digit = atoi(sdigit.c_str());

        PSprite dg_spr;
        dg_spr.setTexture(dmg_spritesheet.t);
        dg_spr.setTextureRect(dmg_spritesheet.get_bounds((digit*3)+type)); ///rect of the specific damage digit from spritesheet
        dg_spr.setOrigin(dg_spr.getLocalBounds().width/2, dg_spr.getLocalBounds().height);

        sf::Vector2f dg_pos(baseX+(i*separator), baseY);

        tmp.spr.push_back(dg_spr);
        tmp.pos.push_back(dg_pos);
        tmp.scale.push_back(dg_scale);
        tmp.scale_goal.push_back(init_scale);
        tmp.mode.push_back(true);
        tmp.alpha.push_back(0);
    }

    dmgCounters.push_back(tmp);
}

void MissionController::addItemsCounter(int id, float baseX, float baseY)
{

}

void MissionController::Initialise(Config &config, std::map<int,bool> &keyMap,std::string backgroundString,V4Core &v4core_)
{
    v4core = &v4core_;
    sf::Context context;
    test_bg.Load(backgroundString, config);//config.GetString("debugBackground"));

    PSprite ps_temp;
    ps_temp.loadFromFile("resources/graphics/item/icon/spear.png",1);
    ps_temp.setRepeated(false);
    ps_temp.setTextureRect(sf::IntRect(0,0,ps_temp.t.getSize().x,ps_temp.t.getSize().y)); ///affect later with ratio
    ps_temp.setOrigin(ps_temp.t.getSize().x,0);
    ps_temp.setColor(sf::Color(255,255,255,255));
    ps_temp.setPosition(0,0);

    s_proj = ps_temp;
    s_proj.scaleX=0.15f;
    s_proj.scaleY=0.15f;

    dmg_spritesheet.load("resources/graphics/mission/damagesheet.png", config.GetInt("textureQuality"), 1);

    ///THIS DOESN'T WORK. CHANGE TO SMART POINTERS.

    ///clean the taken up memory
    delete patapon;
    delete kacheek;
    delete kacheek2;
    delete kacheek3;
    delete endFlag1;
    delete feverworm;
    delete hatapon;

    ///redeclare all objects
    patapon = new Patapon;
    kacheek = new Kacheek;
    kacheek2 = new Kacheek;
    kacheek3 = new Kacheek;
    endFlag1 = new EndFlag;
    feverworm = new FeverWorm;
    hatapon = new Hatapon;

    ///THIS DOESN'T WORK. CHANGE TO SMART POINTERS.



    tangibleLevelObjects.clear();
    units.clear();

    levelProjectiles.clear();

    //ctor
    f_font.loadFromFile("resources/fonts/p4kakupop-pro.ttf");
    //f_font.loadFromFile("resources/fonts/arial.ttf");
    t_timerMenu.setFont(f_font);
    t_timerMenu.setCharacterSize(38);
    t_timerMenu.setFillColor(sf::Color::White);
    //f_font.loadFromFile("resources/fonts/arial.ttf");
    //t_cutscene_text.setFont(f_font);

    //t_cutscene_text.setCharacterSize(35);
    //t_cutscene_text.setFillColor(sf::Color::White);
    //t_cutscene_text.setString(Func::ConvertToUtf8String(config.strRepo.GetUnicodeString(L"intro_cutscene_1")));
    //t_cutscene_text.setOrigin(t_cutscene_text.getGlobalBounds().width/2,t_cutscene_text.getGlobalBounds().height/2);

    missionKeyMap = &keyMap;
    missionConfig = &config;

    isInitialized = true;
    // this is outside the loop
    startAlpha = 255;
    endAlpha = 0;
    targetTime = sf::seconds(10);


    fade.setPosition(sf::Vector2f(0,0));
    fade.setFillColor(sf::Color(0,0,0,0));
    fade.setSize(sf::Vector2f(800,600));
    currentCutsceneId=0;
}
void MissionController::StartMission(std::string songName,int missionID,bool showCutscene)
{
    string missionName = "";
    string missionImg = "";

    sf::Context context;
    int quality = missionConfig->GetInt("textureQuality");
    float ratioX, ratioY;
    patapon->LoadConfig(missionConfig);
    hatapon->LoadConfig(missionConfig);

    army_X=0;
    camera.camera_x=480;
    showTimer = false;

    switch(quality)
    {
        case 0: ///low
        {
            ratioX = missionConfig->GetInt("resX") / float(640);
            ratioY = missionConfig->GetInt("resY") / float(360);
            break;
        }

        case 1: ///med
        {
            ratioX = missionConfig->GetInt("resX") / float(1280);
            ratioY = missionConfig->GetInt("resY") / float(720);
            break;
        }

        case 2: ///high
        {
            ratioX = missionConfig->GetInt("resX") / float(1920);
            ratioY = missionConfig->GetInt("resY") / float(1080);
            break;
        }

        case 3: ///ultra
        {
            ratioX = missionConfig->GetInt("resX") / float(3840);
            ratioY = missionConfig->GetInt("resY") / float(2160);
            break;
        }
    }

    pataponY = 720 - 174;
    floorY = 720 - 100;

    hatapon->setGlobalPosition(sf::Vector2f(200, 720 - 210));

    if(showCutscene)
    {
        cutscene_text_identifiers.push_back(L"intro_cutscene_1");
        cutscene_text_identifiers.push_back(L"intro_cutscene_2");
        cutscene_text_identifiers.push_back(L"intro_cutscene_3");
        cutscene_text_identifiers.push_back(L"intro_cutscene_4");
        cutscene_text_identifiers.push_back(L"intro_cutscene_5");
        cutscene_lengths.push_back(4);
        cutscene_lengths.push_back(4);
        cutscene_lengths.push_back(3);
        cutscene_lengths.push_back(3);
        cutscene_lengths.push_back(2);
        cutscene_blackscreens.push_back(true);
        cutscene_blackscreens.push_back(true);
        cutscene_blackscreens.push_back(true);
        cutscene_blackscreens.push_back(true);
        cutscene_blackscreens.push_back(true);
        currentCutsceneId=0;
        cutscenesLeft=true;
        isFinishedLoading=true;

    }
    else
    {
        inCutscene = false;
        cutscene_blackscreens.clear();
        cutscene_lengths.clear();
        cutscene_text_identifiers.clear();
        cutscenesLeft=false;
    }

    tangibleLevelObjects.clear();
    levelProjectiles.clear();

    switch(missionID)
    {
    case 1:
    {
        showTimer=true;
        endFlag1->LoadConfig(missionConfig);
        feverworm->LoadConfig(missionConfig);
        kacheek->LoadConfig(missionConfig);

        tangibleLevelObjects.push_back(endFlag1);
        tangibleLevelObjects.push_back(feverworm);
        tangibleLevelObjects.push_back(kacheek);

        endFlag1->setGlobalPosition(sf::Vector2f(2500,720 - (180)));
        feverworm->setGlobalPosition(sf::Vector2f(-330,720 - (520)));
        kacheek->setGlobalPosition(sf::Vector2f(1000,720 - (185)));
        kacheek2->setGlobalPosition(sf::Vector2f(1500,720 - (185)));
        kacheek3->setGlobalPosition(sf::Vector2f(2000,720 - (185)));

        missionName = "undefined";
        missionImg = "wasteland";
        break;
    }
    case 2:
    {
        kacheek->LoadConfig(missionConfig);
        kacheek2->LoadConfig(missionConfig);
        kacheek3->LoadConfig(missionConfig);
        feverworm->LoadConfig(missionConfig);
        endFlag1->LoadConfig(missionConfig);
        tangibleLevelObjects.push_back(kacheek);
        tangibleLevelObjects.push_back(kacheek2);
        tangibleLevelObjects.push_back(kacheek3);
        tangibleLevelObjects.push_back(feverworm);
        tangibleLevelObjects.push_back(endFlag1);

        kacheek->setGlobalPosition(sf::Vector2f(1000,720 - (185)));
        kacheek2->setGlobalPosition(sf::Vector2f(1500,720 - (185)));
        kacheek3->setGlobalPosition(sf::Vector2f(2000,720 - (185)));
        feverworm->setGlobalPosition(sf::Vector2f(-330,720 - (520)));
        endFlag1->setGlobalPosition(sf::Vector2f(2500,720 - (180)));

        missionName = "undefined";
        missionImg = "gonrok";
        break;

    }
    default:

        feverworm->LoadConfig(missionConfig);
        endFlag1->LoadConfig(missionConfig);

        tangibleLevelObjects.push_back(feverworm);
        tangibleLevelObjects.push_back(endFlag1);

        endFlag1->setGlobalPosition(sf::Vector2f(2500,720 - (250)));
        feverworm->setGlobalPosition(sf::Vector2f(-330,720 - (520)));

        missionName = "Unspecified Mission";
        missionImg = "wasteland";
        break;
    }

    units.push_back(*patapon);
    units.push_back(*patapon);
    units.push_back(*patapon);

    isFinishedLoading=true;
    v4core->LoadingWaitForKeyPress();

    string fm = "Playing mission: "+missionName;
    v4core->ChangeRichPresence(fm.c_str(), missionImg.c_str(), "logo");
    rhythm.LoadTheme(songName); // missionConfig->GetString("debugTheme")
    missionTimer.restart();
}
void MissionController::StopMission()
{
    rhythm.Stop();
    isInitialized = false;
}
void MissionController::DoKeyboardEvents(sf::RenderWindow &window, float fps, std::map<int,bool> *keyMap, std::map<int,bool> *keyMapHeld)
{
    /// do the keyboard things
    if(rhythm.rhythmController.keyMap[missionConfig->GetInt("keybindSpace")])
    {
        //cout<<"created new projectile. Now there are "<<levelProjectiles.size()+1<<endl;
        unique_ptr<Spear> p = make_unique<Spear>(s_proj);
        p.get()->xPos = patapon->getGlobalPosition().x+patapon->hitBox.left+patapon->hitBox.width/2;
        p.get()->yPos = patapon->getGlobalPosition().y+patapon->hitBox.top+patapon->hitBox.height/2;
        p.get()->speed=1050;
        p.get()->angle=-3.14159*4.0/12; /// 60 degrees from the floor - pi*4/12
        p.get()->maxdmg = 3;
        p.get()->mindmg = 2;
        p.get()->crit = 0;
        p.get()->crit = 0;

        levelProjectiles.push_back(std::move(p));
    }

    if(rhythm.rhythmController.keyMap[sf::Keyboard::T])
    {
        int a = rand() % 3;
        int b = rand() % 9999 + 1;
        int c = rand() % 1280 + 1;
        int d = rand() % 720 + 1;
        addDmgCounter(a, b, c, d);
    }
}

bool MissionController::DoCollisionForObject(HitboxFrame* currentObjectHitBoxFrame,float currentObjectX,float currentObjectY)
{
    for(int i=0; i<tangibleLevelObjects.size(); i++)
    {
        for(int h=0; h<tangibleLevelObjects[i]->hitboxes.size(); h++)
        {
            //cout << "tangibleLevelObjects[" << i << "][" << h << "]" << endl;

            /// NEW COLLISION SYSTEM:
            /// Separating axis theorem
            /// we check an axis at a time
            /// 8 axes in total, aligned with the normal of each face of each shape
            /// thankfully because we are only using rectangles, there are two pairs of parallel sides
            /// so we only need to check 4 axes, as the other 4 are all parallel.
            ///
            /// in each axis we calculate the vector projection onto the axis between the origin and each corner of each box
            /// and find the maximum projection and minimum projection for each shape
            /// then we check if min2>max1 or min1>max2 there has been a collision in this axis
            /// there has to be a collision in ALL axes for actual collision to be confirmed,
            /// so we can stop checking if we find a single non-collision.




            /// axis 1: obj1 "sideways" We start with sideways because it is less likely to contain a collision
            //cout<<"Collision step for projectile at X: "<<currentObjectX<<" against object at x: "<<tangibleLevelObjects[i]->global_x<<endl;

            float currentAxisAngle = currentObjectHitBoxFrame->rotation;
            vector<sf::Vector2f> currentVertices = currentObjectHitBoxFrame->getCurrentVertices();
            PVector pv1 = PVector::getVectorCartesian(0,0,currentVertices[0].x+currentObjectX,currentVertices[0].y+currentObjectY);
            PVector pv2 = PVector::getVectorCartesian(0,0,currentVertices[1].x+currentObjectX,currentVertices[1].y+currentObjectY);
            PVector pv3 = PVector::getVectorCartesian(0,0,currentVertices[2].x+currentObjectX,currentVertices[2].y+currentObjectY);
            PVector pv4 = PVector::getVectorCartesian(0,0,currentVertices[3].x+currentObjectX,currentVertices[3].y+currentObjectY);

            pv1.angle =-atan2(currentVertices[0].y+currentObjectY, currentVertices[0].x+currentObjectX);
            pv2.angle =-atan2(currentVertices[1].y+currentObjectY, currentVertices[1].x+currentObjectX);
            pv3.angle =-atan2(currentVertices[2].y+currentObjectY, currentVertices[2].x+currentObjectX);
            pv4.angle =-atan2(currentVertices[3].y+currentObjectY, currentVertices[3].x+currentObjectX);

            float proj1 = pv1.GetScalarProjectionOntoAxis(currentAxisAngle);
            float proj2 = pv2.GetScalarProjectionOntoAxis(currentAxisAngle);
            float proj3 = pv3.GetScalarProjectionOntoAxis(currentAxisAngle);
            float proj4 = pv4.GetScalarProjectionOntoAxis(currentAxisAngle);

            bool isCollision = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],currentObjectHitBoxFrame,currentObjectX,currentObjectY);
            if (!isCollision)
            {
                continue;
            }
            //cout<<"COLLISION FOUND IN axis 1"<<endl;

            /// axis 2: obj1 "up"
            currentAxisAngle = 3.14159265358/2+currentObjectHitBoxFrame->rotation;
            bool isCollision2 = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],currentObjectHitBoxFrame,currentObjectX,currentObjectY);
            if (!isCollision2)
            {
                continue;
            }
            //cout<<"COLLISION FOUND IN axis 2 (up)"<<endl;

            /// axis 3: obj2 "up" (we add the 90 degrees from before to its current rotation)
            currentAxisAngle = tangibleLevelObjects[i]->hitboxes[h].hitboxObject.rotation + 3.14159265358/2;

            bool isCollision3 = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],currentObjectHitBoxFrame,currentObjectX,currentObjectY);
            if (!isCollision3)
            {
                continue;
            }
            //cout<<"COLLISION FOUND IN axis 3 (up2)"<<endl;

            /// axis 4: obj2 "sideways"
            currentAxisAngle = tangibleLevelObjects[i]->hitboxes[h].hitboxObject.rotation;

            bool isCollision4 = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],currentObjectHitBoxFrame,currentObjectX,currentObjectY);
            if (!isCollision4)
            {
                continue;
            }

            /// we have a collision
            if (isCollision&&isCollision2&&isCollision3&&isCollision4)
            {
                std::cout << "[COLLISION_SYSTEM]: Found a collision"<<endl;
                tangibleLevelObjects[i]->OnCollide(tangibleLevelObjects[i]);
                return true;
            }
            else
            {
                cout<<"Something is very wrong"<<endl;
            }
        }
    }
}
float MissionController::pataponMaxProjection(float axisAngle){
    float currentAxisAngle = 0;
    sf::Vector2f movingObjC1 = sf::Vector2f(patapon->hitBox.left,patapon->hitBox.top); /// "top left"
    sf::Vector2f movingObjC2 = sf::Vector2f(patapon->hitBox.left+patapon->hitBox.width,patapon->hitBox.top); /// "top right"
    sf::Vector2f movingObjC3 = sf::Vector2f(patapon->hitBox.left,patapon->hitBox.top+patapon->hitBox.height); /// "bottom left"
    sf::Vector2f movingObjC4 = sf::Vector2f(patapon->hitBox.left+patapon->hitBox.width,patapon->hitBox.top+patapon->hitBox.height); /// "bottom right"



    HitboxFrame tmp;
    tmp.time = 0;
    tmp.g_x = 0;
    tmp.g_y = 0;
    tmp.clearVertices();
    tmp.addVertex(movingObjC1.x,movingObjC1.y); /// "top left"
    tmp.addVertex(movingObjC2.x,movingObjC2.y); /// "top right"
    tmp.addVertex(movingObjC3.x,movingObjC3.y); /// "bottom left"
    tmp.addVertex(movingObjC4.x,movingObjC4.y); /// "bottom right"
    tmp.rotation = 0;

    std::vector<sf::Vector2f> currentVertices = tmp.getCurrentVertices();

    PVector pv1 = PVector::getVectorCartesian(0,0,currentVertices[0].x+patapon->getGlobalPosition().x,currentVertices[0].y+patapon->getGlobalPosition().y);
    PVector pv2 = PVector::getVectorCartesian(0,0,currentVertices[1].x+patapon->getGlobalPosition().x,currentVertices[1].y+patapon->getGlobalPosition().y);
    PVector pv3 = PVector::getVectorCartesian(0,0,currentVertices[2].x+patapon->getGlobalPosition().x,currentVertices[2].y+patapon->getGlobalPosition().y);
    PVector pv4 = PVector::getVectorCartesian(0,0,currentVertices[3].x+patapon->getGlobalPosition().x,currentVertices[3].y+patapon->getGlobalPosition().y);

    pv1.angle =-atan2(currentVertices[0].y+patapon->getGlobalPosition().y, currentVertices[0].x+patapon->getGlobalPosition().x);
    pv2.angle =-atan2(currentVertices[1].y+patapon->getGlobalPosition().y, currentVertices[1].x+patapon->getGlobalPosition().x);
    pv3.angle =-atan2(currentVertices[2].y+patapon->getGlobalPosition().y, currentVertices[2].x+patapon->getGlobalPosition().x);
    pv4.angle =-atan2(currentVertices[3].y+patapon->getGlobalPosition().y, currentVertices[3].x+patapon->getGlobalPosition().x);

    float proj1 = pv1.GetScalarProjectionOntoAxis(axisAngle);
    float proj2 = pv2.GetScalarProjectionOntoAxis(axisAngle);
    float proj3 = pv3.GetScalarProjectionOntoAxis(axisAngle);
    float proj4 = pv4.GetScalarProjectionOntoAxis(axisAngle);

    /*if(axisAngle!=0){
        cout<<"NEW MAX TEST"<<endl;
        cout<<"Angle: "<<pv1.angle<<" distance: "<<pv1.distance<<" current X: "<<currentVertices[0].x+patapon->x<<" current Y: "<<currentVertices[0].y+patapon->y<<" proj: "<<proj1<<endl;
        cout<<"Angle: "<<pv2.angle<<" distance: "<<pv2.distance<<" current X: "<<currentVertices[1].x+patapon->x<<" current Y: "<<currentVertices[1].y+patapon->y<<" proj: "<<proj2<<endl;
        cout<<"Angle: "<<pv3.angle<<" distance: "<<pv3.distance<<" current X: "<<currentVertices[2].x+patapon->x<<" current Y: "<<currentVertices[2].y+patapon->y<<" proj: "<<proj3<<endl;
        cout<<"Angle: "<<pv4.angle<<" distance: "<<pv4.distance<<" current X: "<<currentVertices[3].x+patapon->x<<" current Y: "<<currentVertices[3].y+patapon->y<<" proj: "<<proj4<<endl;
    }*/
    float maxProjectionObj1 = max(max(max(proj1,proj2),proj3),proj4);
    float minProjectionObj1 = min(min(min(proj1,proj2),proj3),proj4);
    return maxProjectionObj1;
}
float MissionController::pataponMinProjection(float axisAngle){
    float currentAxisAngle = 0;
    sf::Vector2f movingObjC1 = sf::Vector2f(patapon->hitBox.left,patapon->hitBox.top); /// "top left"
    sf::Vector2f movingObjC2 = sf::Vector2f(patapon->hitBox.left+patapon->hitBox.width,patapon->hitBox.top); /// "top right"
    sf::Vector2f movingObjC3 = sf::Vector2f(patapon->hitBox.left,patapon->hitBox.top+patapon->hitBox.height); /// "bottom left"
    sf::Vector2f movingObjC4 = sf::Vector2f(patapon->hitBox.left+patapon->hitBox.width,patapon->hitBox.top+patapon->hitBox.height); /// "bottom right"



    HitboxFrame tmp;
    tmp.time = 0;
    tmp.g_x = 0;
    tmp.g_y = 0;
    tmp.clearVertices();
    tmp.addVertex(movingObjC1.x,movingObjC1.y); /// "top left"
    tmp.addVertex(movingObjC2.x,movingObjC2.y); /// "top right"
    tmp.addVertex(movingObjC3.x,movingObjC3.y); /// "bottom left"
    tmp.addVertex(movingObjC4.x,movingObjC4.y); /// "bottom right"
    tmp.rotation = 0;

    std::vector<sf::Vector2f> currentVertices = tmp.getCurrentVertices();

    PVector pv1 = PVector::getVectorCartesian(0,0,currentVertices[0].x+patapon->getGlobalPosition().x,currentVertices[0].y+patapon->getGlobalPosition().y);
    PVector pv2 = PVector::getVectorCartesian(0,0,currentVertices[1].x+patapon->getGlobalPosition().x,currentVertices[1].y+patapon->getGlobalPosition().y);
    PVector pv3 = PVector::getVectorCartesian(0,0,currentVertices[2].x+patapon->getGlobalPosition().x,currentVertices[2].y+patapon->getGlobalPosition().y);
    PVector pv4 = PVector::getVectorCartesian(0,0,currentVertices[3].x+patapon->getGlobalPosition().x,currentVertices[3].y+patapon->getGlobalPosition().y);
    pv1.angle =-atan2(currentVertices[0].y+patapon->getGlobalPosition().y, currentVertices[0].x+patapon->getGlobalPosition().x);
    pv2.angle =-atan2(currentVertices[1].y+patapon->getGlobalPosition().y, currentVertices[1].x+patapon->getGlobalPosition().x);
    pv3.angle =-atan2(currentVertices[2].y+patapon->getGlobalPosition().y, currentVertices[2].x+patapon->getGlobalPosition().x);
    pv4.angle =-atan2(currentVertices[3].y+patapon->getGlobalPosition().y, currentVertices[3].x+patapon->getGlobalPosition().x);

    float proj1 = pv1.GetScalarProjectionOntoAxis(axisAngle);
    float proj2 = pv2.GetScalarProjectionOntoAxis(axisAngle);
    float proj3 = pv3.GetScalarProjectionOntoAxis(axisAngle);
    float proj4 = pv4.GetScalarProjectionOntoAxis(axisAngle);


    float maxProjectionObj1 = max(max(max(proj1,proj2),proj3),proj4);
    float minProjectionObj1 = min(min(min(proj1,proj2),proj3),proj4);
    return minProjectionObj1;
}
bool MissionController::DoCollisionStepInAxis(float currentAxisAngle,HitboxFrame* currentHitboxFrame,AnimatedObject* targetObject, HitboxFrame* currentObjectHitBoxFrame,float currentObjectX,float currentObjectY)
{
    std::vector<sf::Vector2f> currentVertices = currentObjectHitBoxFrame->getCurrentVertices();
    PVector pv1 = PVector::getVectorCartesian(0,0,currentVertices[0].x+currentObjectX,currentVertices[0].y+currentObjectY);
    PVector pv2 = PVector::getVectorCartesian(0,0,currentVertices[1].x+currentObjectX,currentVertices[1].y+currentObjectY);
    PVector pv3 = PVector::getVectorCartesian(0,0,currentVertices[2].x+currentObjectX,currentVertices[2].y+currentObjectY);
    PVector pv4 = PVector::getVectorCartesian(0,0,currentVertices[3].x+currentObjectX,currentVertices[3].y+currentObjectY);
    pv1.angle =-atan2(currentVertices[0].y+currentObjectY, currentVertices[0].x+currentObjectX);
    pv2.angle =-atan2(currentVertices[1].y+currentObjectY, currentVertices[1].x+currentObjectX);
    pv3.angle =-atan2(currentVertices[2].y+currentObjectY, currentVertices[2].x+currentObjectX);
    pv4.angle =-atan2(currentVertices[3].y+currentObjectY, currentVertices[3].x+currentObjectX);

    float proj1 = pv1.GetScalarProjectionOntoAxis(currentAxisAngle);
    float proj2 = pv2.GetScalarProjectionOntoAxis(currentAxisAngle);
    float proj3 = pv3.GetScalarProjectionOntoAxis(currentAxisAngle);
    float proj4 = pv4.GetScalarProjectionOntoAxis(currentAxisAngle);


    float maxProjectionObj1 = max(max(max(proj1,proj2),proj3),proj4);
    float minProjectionObj1 = min(min(min(proj1,proj2),proj3),proj4);

    float maxProjectionObj2 = currentHitboxFrame->maxProjection(currentAxisAngle, targetObject->global_x,targetObject->global_y);
    float minProjectionObj2 = currentHitboxFrame->minProjection(currentAxisAngle, targetObject->global_x,targetObject->global_y);
    if(maxProjectionObj1>minProjectionObj2 && minProjectionObj1<maxProjectionObj2)
    {
        return true;
    }
    else
    {
        return false;
    }


}
void MissionController::DoMovement(sf::RenderWindow &window, float fps, std::map<int,bool> *keyMap, std::map<int,bool> *keyMapHeld)
{
    /** Make Patapon walk (temporary) **/
    float booster=1.0;
    if (rhythm.current_perfect == 4)
    {
        booster=1.2;
    }
    //cout << "camera.walk: " << camera.walk << endl;

    if(camera.walk)
    {
        int farthest_id = -1;
        float temp_pos = -9999;
        for(int i=0; i<units.size(); i++)
        {
            if(temp_pos <= units[i].getGlobalPosition().x)
            {
                temp_pos = units[i].getGlobalPosition().x;
                farthest_id = i;
            }
        }

        float proposedXPos = units[farthest_id].global_x + (2 * 60 * booster) / fps;
        //cout << "global_x: " << units[farthest_id].global_x << endl;
        //cout << "proposedXPos = " << proposedXPos << endl;
        /// use the right hand side of the patapon sprite to check for collisions. This should be changed if the patapon walks to the left
        //float proposedXPosRight = proposedXPos + units[farthest_id].hitBox.left + units[farthest_id].hitBox.width;
        /// need to have it check for collision and stop if blocked by kacheek here.

        /// right now it is very basic checking only in X axis. Jumping over a
        /// kacheek will not be possible.

        bool foundCollision = false;
        for(int i=0; i<tangibleLevelObjects.size(); i++)
        {
            for(int h=0; h<tangibleLevelObjects[i]->hitboxes.size(); h++)
            {
                //cout << "tangibleLevelObjects[" << i << "][" << h << "]" << endl;

                /// NEW COLLISION SYSTEM:
                /// Separating axis theorem
                /// we check an axis at a time
                /// 8 axes in total, aligned with the normal of each face of each shape
                /// thankfully because we are only using rectangles, there are two pairs of parallel sides
                /// so we only need to check 4 axes, as the other 4 are all parallel.
                ///
                /// in each axis we calculate the vector projection onto the axis between the origin and each corner of each box
                /// and find the maximum projection and minimum projection for each shape
                /// then we check if min2>max1 or min1>max2 there has been a collision in this axis
                /// there has to be a collision in ALL axes for actual collision to be confirmed,
                /// so we can stop checking if we find a single non-collision.




                /// axis 1: obj1 "sideways" We start with sideways because it is less likely to contain a collision
                float currentAxisAngle = 0;
                sf::Vector2f movingObjC1 = sf::Vector2f(units[farthest_id].hitBox.left,units[farthest_id].hitBox.top); /// "top left"
                sf::Vector2f movingObjC2 = sf::Vector2f(units[farthest_id].hitBox.left+units[farthest_id].hitBox.width,units[farthest_id].hitBox.top); /// "top right"
                sf::Vector2f movingObjC3 = sf::Vector2f(units[farthest_id].hitBox.left,units[farthest_id].hitBox.top+units[farthest_id].hitBox.height); /// "bottom left"
                sf::Vector2f movingObjC4 = sf::Vector2f(units[farthest_id].hitBox.left+units[farthest_id].hitBox.width,units[farthest_id].hitBox.top+units[farthest_id].hitBox.height); /// "bottom right"



                HitboxFrame tmp;
                tmp.time = 0;
                tmp.g_x = 0;
                tmp.g_y = 0;
                tmp.clearVertices();
                tmp.addVertex(movingObjC1.x,movingObjC1.y); /// "top left"
                tmp.addVertex(movingObjC2.x,movingObjC2.y); /// "top right"
                tmp.addVertex(movingObjC3.x,movingObjC3.y); /// "bottom left"
                tmp.addVertex(movingObjC4.x,movingObjC4.y); /// "bottom right"
                tmp.rotation = 0;


                bool isCollision = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],&tmp,proposedXPos,units[farthest_id].getGlobalPosition().y);
                if (!isCollision)
                {
                    continue;
                }
                //cout<<"COLLISION FOUND IN axis 1"<<endl;

                /// axis 2: obj1 "up"
                currentAxisAngle = 3.14159265358/2;
                bool isCollision2 = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],&tmp,proposedXPos,units[farthest_id].getGlobalPosition().y);
                if (!isCollision2)
                {
                    continue;
                }
                //cout<<"COLLISION FOUND IN axis 2 (up)"<<endl;

                /// axis 3: obj2 "up" (we add the 90 degrees from before to its current rotation)
                currentAxisAngle = tangibleLevelObjects[i]->hitboxes[h].hitboxObject.rotation + currentAxisAngle;

                bool isCollision3 = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],&tmp,proposedXPos,units[farthest_id].getGlobalPosition().y);
                if (!isCollision3)
                {
                    continue;
                }
                //cout<<"COLLISION FOUND IN axis 3 (up2)"<<endl;

                /// axis 4: obj2 "sideways"
                currentAxisAngle = tangibleLevelObjects[i]->hitboxes[h].hitboxObject.rotation;

                bool isCollision4 = DoCollisionStepInAxis(currentAxisAngle,&(tangibleLevelObjects[i]->hitboxes[h].hitboxObject),tangibleLevelObjects[i],&tmp,proposedXPos,units[farthest_id].getGlobalPosition().y);
                if (!isCollision4)
                {
                    continue;
                }






                /// we have a collision
                if (isCollision&&isCollision2&&isCollision3&&isCollision4)
                {
                    foundCollision = true;
                    tangibleLevelObjects[i]->OnCollide(tangibleLevelObjects[i]);
                    std::cout << "[COLLISION_SYSTEM]: Found a collision"<<endl;
                }
                else
                {
                    cout<<"Something is very wrong"<<endl;
                }
            }
        }

        /// if the new position is inside a kacheek, don't move. If we found anything,
        if (!foundCollision)
        {
            army_X += proposedXPos - units[farthest_id].global_x;
        }
    }

    for(int i=0; i<units.size(); i++)
    {
        units[i].setGlobalPosition(sf::Vector2f(army_X+100+(50*i),pataponY));
        units[i].fps = fps;

        //if(rhythm.current_song == "patapata")
        //{
        //    units[i].current_animation = "walk";
        //}
    }

    /// step 1: all projectiles have gravity applied to them
    for(int i=0; i<levelProjectiles.size(); i++)
    {
        Projectile* p = levelProjectiles[i].get();
        float xspeed = p->GetXSpeed();
        float yspeed = p->GetYSpeed();
        yspeed += (gravity/fps);
        p->SetNewSpeedVector(xspeed,yspeed);
        p->Update(window,fps);
    }
    /// step 2: any projectiles that hit the floor are destroyed
    for(int i=0; i<levelProjectiles.size(); i++)
    {
        Projectile* p = levelProjectiles[i].get();
        float ypos = p->yPos;
        if (ypos>floorY)
        {
            levelProjectiles.erase(levelProjectiles.begin()+i);
        }
    }
    /// step 3: any projectiles that hit any collidableobject are informed
    for(int i=0; i<levelProjectiles.size(); i++)
    {
        Projectile* p = levelProjectiles[i].get();
        float ypos = p->yPos;
        float xpos = p->xPos;
        HitboxFrame tmp;
        tmp.time = 0;
        tmp.g_x = 0;
        tmp.g_y = 0;
        tmp.clearVertices();
        tmp.addVertex(-3,-1); /// "top left"
        tmp.addVertex(3,-1); /// "top right"
        tmp.addVertex(-3,1); /// "bottom left"
        tmp.addVertex(3,1); /// "bottom right"
        tmp.rotation = -p->angle;
        if (DoCollisionForObject(&tmp,xpos,ypos)){
            levelProjectiles.erase(levelProjectiles.begin()+i);
        };
    }

}
void MissionController::Update(sf::RenderWindow &window, float fps, std::map<int,bool> *keyMap,std::map<int,bool> *keyMapHeld)
{
    if(rhythm.current_song == "patapata")
    {
        //cout << "set walk true" << endl;
        camera.walk = true;
    }
    else
    {
        //cout << "set walk false" << endl;
        camera.walk = false;
    }

    missionKeyMap = keyMap;

    int quality = 1;
    float ratioX=1,ratioY=1;

    switch(quality)
    {
        case 0: ///low
        {
            ratioX = window.getSize().x / float(640);
            ratioY = window.getSize().y / float(360);
            break;
        }

        case 1: ///med
        {
            ratioX = window.getSize().x / float(1280);
            ratioY = window.getSize().y / float(720);
            break;
        }

        case 2: ///high
        {
            ratioX = window.getSize().x / float(1920);
            ratioY = window.getSize().y / float(1080);
            break;
        }

        case 3: ///ultra
        {
            ratioX = window.getSize().x / float(3840);
            ratioY = window.getSize().y / float(2160);
            break;
        }
    }

    camera.followobject_x = hatapon->getGlobalPosition().x * ratioX;
    camera.Work(window,fps,keyMapHeld);
    test_bg.setCamera(camera);
    test_bg.Draw(window);

    kacheek->fps = fps;
    kacheek2->fps = fps;
    kacheek3->fps = fps;
    endFlag1->fps = fps;
    feverworm->fps = fps;

    if(rhythm.GetCombo() >= 11)
    {
        if(feverworm->getAnimationSegment() == "fever")
        {
            feverworm->scaleX = 1+rhythm.r_gui.beatBounce;
            feverworm->scaleY = 1+rhythm.r_gui.beatBounce;
        }
    }

    if(rhythm.updateworm)
    {
        feverworm->combo = rhythm.GetRealCombo();

            if(rhythm.GetRealCombo() < 2)
            {
                feverworm->global_x = -400;
                feverworm->next_x = -400;
                feverworm->speed = 120;
            }

            if(rhythm.GetRealCombo() == 2)
            {
                feverworm->next_x = -70;
                feverworm->speed = 400;
            }

            if((rhythm.GetRealCombo() > 2) && (rhythm.GetCombo() < 11))
            {
                if(rhythm.advanced_prefever)
                feverworm->next_x = -50 + (rhythm.GetSatisfaction() / 5.5) + ((rhythm.GetRealCombo() - 2) * 8);
                else
                feverworm->next_x = -70 + ((rhythm.GetRealCombo() - 2) * 8);

                feverworm->speed = 40;
            }

            if(rhythm.GetCombo() < 11)
            {
                if(rhythm.advanced_prefever)
                {
                    feverworm->setAnimationSegment("fast");
                }
                else
                {
                    feverworm->setAnimationSegment("slow");
                }
            }

            if(rhythm.GetCombo() == 11)
            {
                feverworm->setAnimationSegment("transform");
                feverworm->setLoop(false);
            }

            if(rhythm.GetCombo() >= 12)
            {
                feverworm->setAnimationSegment("fever");
                feverworm->setLoop(true);
            }

            rhythm.updateworm = false;
    }


    hatapon->setGlobalPosition(sf::Vector2f(army_X,494));
    hatapon->fps = fps;
    DoKeyboardEvents(window,fps,keyMap,keyMapHeld);
    DoMovement(window,fps,keyMap,keyMapHeld);

    // TODO: at some point some pointer shenanigans is required to make these be a reference to v4core's ones too.
    rhythm.rhythmController.keyMap = *missionKeyMap;
    rhythm.rhythmController.config = *missionConfig;
    rhythm.config = *missionConfig;



    if((rhythm.current_song != "") && ((rhythm.current_song != "dondon") && (rhythm.current_song != "ponpata")))
    {
        hatapon->setAnimationSegment("wave");
    }
    else if(rhythm.current_song == "dondon")
    {
        hatapon->setAnimationSegment("jump");
    }
    else if(rhythm.current_song == "ponpata")
    {
        hatapon->setAnimationSegment("flee");
    }
    else
    {
        hatapon->setAnimationSegment("idle");
    }

    for (int i=0; i<tangibleLevelObjects.size(); i++)
    {
        tangibleLevelObjects[i]->Draw(window);
    }

    int farthest_id = -1;
    float temp_pos = -9999;
    for(int i=0; i<units.size(); i++)
    {
        if(temp_pos <= units[i].getGlobalPosition().x)
        {
            temp_pos = units[i].getGlobalPosition().x;
            farthest_id = i;
        }
    }

    for(int i=0; i<units.size(); i++)
    {
        if(rhythm.current_song == "patapata")
        {
            units[i].action = units[i].WALK;

            if(!units[i].focus)
            units[i].setAnimationSegment("walk");
            else
            units[i].setAnimationSegment("walk_focused");

            units[i].setLoop(true);
        }
        else
        {
            if(units[i].getAnimationSegment() == "walk")
            {
                if(!units[i].getback)
                {
                    if(!units[i].focus)
                    units[i].setAnimationSegment("idle_armed", true);
                    else
                    units[i].setAnimationSegment("idle_armed_focused", true);
                }
            }
        }

        if(rhythm.current_song == "ponpon")
        {
            units[i].startAttack();
        }
        else
        {
            if(units[i].action == units[i].ATTACK)
            {
                units[i].action = 0;

                if(units[i].getAnimationSegment() == "attack")
                units[i].setAnimationSegment("idle_armed", true);
            }
        }

        if((rhythm.rhythmController.current_drum == "pata") or (rhythm.rhythmController.current_drum == "pon") or (rhythm.rhythmController.current_drum == "chaka") or (rhythm.rhythmController.current_drum == "don"))
        {
            //cout << rhythm.rhythmController.current_drum << endl;

            if(!units[i].focus)
            units[i].setAnimationSegment(rhythm.rhythmController.current_drum, true);
            else
            units[i].setAnimationSegment(rhythm.rhythmController.current_drum+"_focused", true);
        }

        vector<float> gdistances;
        vector<float> ldistances;

        for (int e=0; e<tangibleLevelObjects.size(); e++)
        {
            //cout << "tangibleLevelObjects[" << i << "]->Draw(window);" << endl;
            ///some temporary code for entity in range detection
            if(tangibleLevelObjects[e]->type == tangibleLevelObjects[e]->HOSTILE)
            {
                float global_distance = tangibleLevelObjects[e]->getGlobalPosition().x - (units[farthest_id].getGlobalPosition().x - units[farthest_id].local_x);
                float local_distance = tangibleLevelObjects[e]->getGlobalPosition().x - units[farthest_id].getGlobalPosition().x;
                gdistances.push_back(global_distance);
                ldistances.push_back(local_distance);
            }
        }

        /// patapons (and other enemies) are drawn after level objects like kacheek so they are always on top
        if(units[i].threw)
        {
            float rand_hs = (rand() % 1000) / float(10);
            float rand_vs = (rand() % 1000) / float(10);

            float rand_rad = (rand() % 200000000) / float(1000000000);

            //cout << "===============================================" << endl;
            //cout << "rand_hs: " << rand_hs << " rand_vs: " << rand_vs << endl;
            //cout << "===============================================" << endl;

            unique_ptr<Spear> p = make_unique<Spear>(s_proj);
            p.get()->xPos = units[i].getGlobalPosition().x+units[i].hitBox.left+units[i].hitBox.width/2-26;
            p.get()->yPos = units[i].getGlobalPosition().y+units[i].hitBox.top+units[i].hitBox.height/2-35;
            p.get()->speed = 800;
            p.get()->hspeed = 450+rand_hs;
            p.get()->vspeed = -450+rand_vs;
            ///-0.523598776 - 30 deg
            //p.get()->angle = -0.698131701; /// 40 degrees from the floor - pi*4/12
            p.get()->angle = -0.58 - rand_rad;
            ///-0.872664626 - 50 deg
            p.get()->maxdmg = 3;
            p.get()->mindmg = 2;
            p.get()->crit = 0;
            p.get()->crit = 0;

            levelProjectiles.push_back(std::move(p));

            units[i].threw = false;
        }

        units[i].gclosest = *std::min_element(std::begin(gdistances), std::end(gdistances));
        units[i].lclosest = *std::min_element(std::begin(ldistances), std::end(ldistances));

        units[i].Draw(window);
    }

    if((rhythm.rhythmController.current_drum == "pata") or (rhythm.rhythmController.current_drum == "pon") or (rhythm.rhythmController.current_drum == "chaka") or (rhythm.rhythmController.current_drum == "don"))
    {
        rhythm.rhythmController.current_drum = "";
        rhythm.current_song = "";
    }

    hatapon->Draw(window);

    for(int i=0; i<levelProjectiles.size(); i++)
    {
        levelProjectiles[i].get()->Draw(window,fps);
    }
    /// draw static UI elements
    auto lastView = window.getView();

    window.setView(window.getDefaultView());


    if(cutscenesLeft && !inCutscene && isMoreCutscenes())
    {
        StartCutscene(cutscene_text_identifiers[currentCutsceneId],cutscene_blackscreens[currentCutsceneId],cutscene_lengths[currentCutsceneId]);
    }

    sf::Time currentTime = timer.getElapsedTime();
    int currentAlpha = startAlpha;
    if (currentTime >= targetTime && inCutscene)
    {
        // oops: currentAlpha = endAlpha; // make certain that the alpha is at its final destination
        //you are done
        if(!isMoreCutscenes())
        {
            currentAlpha = startAlpha;
            inCutscene = false;
            if(isBlackScreenCutscene)
            {
                inFadeTransition = true;
                timer.restart();
                targetTime = sf::seconds(2);
            }
            else
            {
                FinishLastCutscene();
            }
            cutscenesLeft=false;
        }
        else
        {
            /// next cutscene
            currentCutsceneId++;
            StartCutscene(cutscene_text_identifiers[currentCutsceneId],cutscene_blackscreens[currentCutsceneId],cutscene_lengths[currentCutsceneId]);
        }
    }
    else if (currentTime >= targetTime && !inCutscene && inFadeTransition)
    {
        currentAlpha = endAlpha;
        inFadeTransition = false;
        FinishLastCutscene();
    }
    else if (!inCutscene && inFadeTransition)
    {
        currentAlpha = startAlpha + (endAlpha - startAlpha) * (currentTime.asMilliseconds() / (targetTime.asMilliseconds()+0.0));
    }
    else if (inCutscene && isBlackScreenCutscene)
    {
        currentAlpha = startAlpha;
    }
    else if (inCutscene)
    {
        currentAlpha = startAlpha;
    }
    // apply alpha to whatever colour is previously set
    if((inFadeTransition || inCutscene) && isBlackScreenCutscene)
    {
        sf::Color fadeColor = fade.getFillColor();
        fadeColor.a = currentAlpha;
        fade.setFillColor(fadeColor);
        fade.setSize(sf::Vector2f(window.getSize().x,window.getSize().y));

        fade.setPosition(0,0);
        window.draw(fade);
    }
    if (inCutscene)
    {
        for (int i=0; i<t_cutscene_text.size(); i++)
        {
            sf::Text currentLine = t_cutscene_text[i];

            currentLine.setPosition(window.getSize().x/2,300 + 39*i);
            sf::Time currentTime = timer.getElapsedTime();

            window.draw(currentLine);
        }
    }
    window.setView(lastView);

    /// here we show the hitbox
    bool showHitboxes = false;
    if(showHitboxes)
    {
        sf::RectangleShape hitboxRect(sf::Vector2f(patapon->hitBox.width, patapon->hitBox.height));
        hitboxRect.setPosition(patapon->getGlobalPosition().x+patapon->hitBox.left,patapon->getGlobalPosition().y+patapon->hitBox.top);
        //window.draw(hitboxRect);

        float minProjectionObj2 = pataponMinProjection(0);
        sf::CircleShape markerr(5);
        markerr.setFillColor(sf::Color(250, 250, 50));
        markerr.setPosition(0-2.5,0-2.5);
        window.draw(markerr);
        markerr.setFillColor(sf::Color(250, 250, 50));
        markerr.setPosition(minProjectionObj2-2.5,patapon->getGlobalPosition().y-2.5+patapon->hitBox.top+patapon->hitBox.height/2);
        window.draw(markerr);

        float maxProjectionObj2 = pataponMaxProjection(0);
        markerr.setFillColor(sf::Color(150, 50, 250));
        markerr.setPosition(maxProjectionObj2-2.5,patapon->getGlobalPosition().y-2.5+patapon->hitBox.top+patapon->hitBox.height/2);
        window.draw(markerr);

        float angled = 3.14159265358/2;
        float minProjectionObj22 = pataponMinProjection(angled);

        markerr.setFillColor(sf::Color(250, 250, 50));
        markerr.setPosition(patapon->getGlobalPosition().x-2.5+patapon->hitBox.left+patapon->hitBox.width/2,minProjectionObj22-2.5);
        window.draw(markerr);


        float maxProjectionObj22 = pataponMaxProjection(angled);
        markerr.setFillColor(sf::Color(150, 50, 250));
        markerr.setPosition(patapon->getGlobalPosition().x-2.5+patapon->hitBox.left+patapon->hitBox.width/2,maxProjectionObj22-2.5);
        window.draw(markerr);

        /*
        sf::Vector2f movingObjC1 = sf::Vector2f(patapon->x+patapon->hitBox.left,patapon->y+patapon->hitBox.top); /// "top left"
        sf::CircleShape shape(5);
        shape.setFillColor(sf::Color(100, 250, 50));
        shape.setPosition(movingObjC1.x-2.5,movingObjC1.y-2.5);
        window.draw(shape);
        sf::Vector2f movingObjC2 = sf::Vector2f(patapon->x+patapon->hitBox.left+patapon->hitBox.width,patapon->y+patapon->hitBox.top); /// "top right"
        shape.setPosition(movingObjC2.x-2.5,movingObjC2.y-2.5);
        window.draw(shape);
        sf::Vector2f movingObjC3 = sf::Vector2f(patapon->x+patapon->hitBox.left,patapon->y+patapon->hitBox.top+patapon->hitBox.height); /// "bottom left"
        shape.setPosition(movingObjC3.x-2.5,movingObjC3.y-2.5);
        window.draw(shape);
        sf::Vector2f movingObjC4 = sf::Vector2f(patapon->x+patapon->hitBox.left+patapon->hitBox.width,patapon->y+patapon->hitBox.top+patapon->hitBox.height); /// "bottom right"
        shape.setPosition(movingObjC4.x-2.5,movingObjC4.y-2.5);
        window.draw(shape);*/

        for(int i=0; i<tangibleLevelObjects.size(); i++)
        {
            for(int h=0; h<tangibleLevelObjects[i]->hitboxes.size(); h++)
            {
                //cout << "hitbox " << h << " pos: " << tangibleLevelObjects[i]->hitboxes[h].getGlobalPosition().x << " " << tangibleLevelObjects[i]->hitboxes[h].getGlobalPosition().y << " " << tangibleLevelObjects[i]->hitboxes[h].getRect().top << " " << tangibleLevelObjects[i]->hitboxes[h].getRect().left << " " << tangibleLevelObjects[i]->hitboxes[h].getRect().width << " " << tangibleLevelObjects[i]->hitboxes[h].getRect().height << endl;
                // create an empty shape

                HitboxFrame* currentHitbox = &(tangibleLevelObjects[i]->hitboxes[h].hitboxObject);
                sf::ConvexShape convex;
                convex.setFillColor(sf::Color(150, 50, 250));
                // resize it to 5 points
                std::vector<sf::Vector2f> currentVertices = currentHitbox->getCurrentVertices();
                convex.setPointCount(currentVertices.size());

                for (int j=0; j<currentVertices.size(); j++)
                {
                    sf::Vector2f currentPoint = currentVertices[j];
                    currentPoint.x = currentPoint.x + currentHitbox->g_x + tangibleLevelObjects[i]->global_x;
                    currentPoint.y = currentPoint.y + currentHitbox->g_y + tangibleLevelObjects[i]->global_y;
                    //cout<<"DRAWING POINT: "<<currentVertices.size()<<" x: "<<currentPoint.x<<" y: "<<currentPoint.y<<endl;
                    sf::CircleShape shape(5);
                    shape.setFillColor(sf::Color(100, 250, 50));
                    shape.setPosition(currentPoint.x-2.5,currentPoint.y-2.5);
                    window.draw(shape);
                    convex.setPoint(i, currentPoint);
                    //cout << "convex.setPoint(" << i << ", " << currentPoint.x << " " << currentPoint.y << ");" << endl;
                }

                float minProjectionObj2 = currentHitbox->minProjection(0, tangibleLevelObjects[i]->global_x,tangibleLevelObjects[i]->global_y);
                sf::CircleShape marker(5);
                marker.setFillColor(sf::Color(250, 50, 150));
                marker.setPosition(minProjectionObj2-2.5,tangibleLevelObjects[i]->global_y-2.5);
                window.draw(marker);


                float maxProjectionObj2 = currentHitbox->maxProjection(0, tangibleLevelObjects[i]->global_x,tangibleLevelObjects[i]->global_y);
                marker.setFillColor(sf::Color(150, 50, 250));
                marker.setPosition(maxProjectionObj2-2.5,tangibleLevelObjects[i]->global_y-2.5);
                window.draw(marker);

                float angled = 3.14159265358/2;
                float minProjectionObj22 = currentHitbox->minProjection(angled, tangibleLevelObjects[i]->global_x,tangibleLevelObjects[i]->global_y);

                marker.setFillColor(sf::Color(250, 50, 150));
                marker.setPosition(tangibleLevelObjects[i]->global_x-2.5,minProjectionObj22-2.5);
                window.draw(marker);


                float maxProjectionObj22 = currentHitbox->maxProjection(angled, tangibleLevelObjects[i]->global_x,tangibleLevelObjects[i]->global_y);
                marker.setFillColor(sf::Color(150, 50, 250));
                marker.setPosition(tangibleLevelObjects[i]->global_x-2.5,maxProjectionObj22-2.5);
                window.draw(marker);

                window.draw(convex);
                ///sf::RectangleShape kacheekHitboxRect(sf::Vector2f(tangibleLevelObjects[i]->hitboxes[h].getRect().width, tangibleLevelObjects[i]->hitboxes[h].getRect().height));
                ///kacheekHitboxRect.setPosition(tangibleLevelObjects[i]->getGlobalPosition().x+tangibleLevelObjects[i]->hitboxes[h].getGlobalPosition().x+tangibleLevelObjects[i]->hitboxes[h].getRect().left,tangibleLevelObjects[i]->getGlobalPosition().y+tangibleLevelObjects[i]->hitboxes[h].getGlobalPosition().y+tangibleLevelObjects[i]->hitboxes[h].getRect().top);
                ///window.draw(kacheekHitboxRect);
            }
        }
    }

    vector<int> dmg_rm;

    for(int i=0; i<dmgCounters.size(); i++)
    {
        float a=0;

        for(int d=0; d<dmgCounters[i].spr.size(); d++)
        {
            if(dmgCounters[i].display_timer.getElapsedTime().asMilliseconds() > 70*d)
            {
                float curScale = dmgCounters[i].scale[d];
                float destScale = dmgCounters[i].scale_goal[d];

                if(dmgCounters[i].mode[d])
                {
                    curScale -= float(14) / fps;
                    dmgCounters[i].alpha[d] += float(1800) / fps;

                    if(curScale <= destScale)
                    {
                        dmgCounters[i].mode[d] = false;
                        destScale = 1;
                    }

                    if(dmgCounters[i].alpha[d] >= 255)
                    dmgCounters[i].alpha[d] = 255;
                }

                if(!dmgCounters[i].mode[d])
                {
                    if(!dmgCounters[i].mode[d])
                    {
                        curScale += float(8) / fps;

                        if(curScale >= destScale)
                        {
                            curScale = destScale;
                        }
                    }
                }

                if(dmgCounters[i].display_timer.getElapsedTime().asMilliseconds() > 70*d + 1000)
                {
                    if(!dmgCounters[i].mode[d])
                    {
                        dmgCounters[i].pos[d].y += float(60) / fps;
                        dmgCounters[i].alpha[d] -= float(500) / fps;

                        if(dmgCounters[i].alpha[d] <= 0)
                        dmgCounters[i].alpha[d] = 0;
                    }
                }

                dmgCounters[i].scale[d] = curScale;
                dmgCounters[i].scale_goal[d] = destScale;

                dmgCounters[i].spr[d].setPosition(dmgCounters[i].pos[d].x, dmgCounters[i].pos[d].y-((curScale-1)*10));
                dmgCounters[i].spr[d].setScale(curScale, curScale);
                dmgCounters[i].spr[d].setColor(sf::Color(255,255,255,dmgCounters[i].alpha[d]));

                dmgCounters[i].spr[d].draw(window);

                a += dmgCounters[i].alpha[d];
            }
        }

        if(a <= 1)
        dmg_rm.push_back(i);
    }

    for(int i=0; i<dmg_rm.size(); i++)
    {
        cout << "Erased dmgCounter " << dmg_rm[i] << endl;
        dmgCounters.erase(dmgCounters.begin()+(dmg_rm[i] - i));
    }

    rhythm.fps = fps;

    if(showTimer)
    {
        auto lastView2 = window.getView();

        window.setView(window.getDefaultView());
        t_timerMenu.setString(Func::ConvertToUtf8String(std::to_string(missionTimer.getElapsedTime().asSeconds())+" Seconds"));
        t_timerMenu.setOrigin(t_timerMenu.getGlobalBounds().width/2,t_timerMenu.getGlobalBounds().height/2);
        t_timerMenu.setPosition(window.getSize().x/2,100);
        window.draw(t_timerMenu);
        window.setView(lastView2);
    }
    rhythm.Draw(window);

}
void MissionController::FinishLastCutscene()
{
    /// runs when the last cutscene of a sequence is done
}
bool MissionController::isMoreCutscenes()
{
    /// returns true if there are more cutscenes
    return currentCutsceneId<cutscene_text_identifiers.size()-1;
}
void MissionController::StartCutscene(const std::wstring& text,bool isBlackScreen, int TimeToShow)
{
    /// because the description needs to be able to go over multiple lines, we have to split it into a series of lines
    t_cutscene_text.clear();
    std::vector<std::wstring> wordsinDesc = Func::Split(missionConfig->strRepo.GetUnicodeString(text),' ');
    sf::String oldTotalString;
    sf::String currentTotalString;
    int maxWidth = missionConfig->GetInt("resX") * 0.4;
    timer.restart();
    inCutscene = true;
    isBlackScreenCutscene = isBlackScreen;
    targetTime = sf::seconds(TimeToShow);
    /// we split it into words, then go word by word testing the width of the string
    for (int i=0; i<wordsinDesc.size(); i++)
    {
        std::wstring currentWord = wordsinDesc[i];
        currentTotalString = currentTotalString + Func::ConvertToUtf8String(currentWord) + L" ";
        sf::Text t_newLine;
        t_newLine.setFont(f_font);
        t_newLine.setCharacterSize(35);
        t_newLine.setFillColor(sf::Color::White);
        t_newLine.setString(currentTotalString);
        if (t_newLine.getGlobalBounds().width>maxWidth)
        {
            /// when the string is too long, we go back to the last string and lock it in, then start a new line
            currentTotalString = oldTotalString;
            t_newLine.setString(currentTotalString);
            t_newLine.setOrigin(t_newLine.getGlobalBounds().width/2,t_newLine.getGlobalBounds().height/2);
            t_cutscene_text.push_back(t_newLine);
            oldTotalString = currentWord+L" ";
            currentTotalString = currentWord+L" ";
        }
        oldTotalString = currentTotalString;
        /// if there are no more words, finish up the current line
        if (i+1==wordsinDesc.size())
        {
            currentTotalString = oldTotalString;
            t_newLine.setString(currentTotalString);
            t_newLine.setOrigin(t_newLine.getGlobalBounds().width/2,t_newLine.getGlobalBounds().height/2);
            t_cutscene_text.push_back(t_newLine);
            oldTotalString = "";
            currentTotalString = "";
        }
    }
}
MissionController::~MissionController()
{
    //dtor

}
