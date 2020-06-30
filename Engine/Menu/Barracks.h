#ifndef BARRACKSMENU_H
#define BARRACKSMENU_H
#include <SFML/Graphics.hpp>
#include "../Config.h"
#include "../Graphics/Menu.h"
#include "../Graphics/PText.h"
#include "../Mission/MissionController.h"

class V4Core;
class Barracks : public Menu
{
    public:
        MissionController* currentController;
        Config *config;
        std::map<int,bool> *keyMap;
        std::string mission_file;

        sf::RectangleShape rs_cover;
        sf::RectangleShape mm_bigBox;
        sf::RectangleShape mm_titleBox;
        sf::RectangleShape mm_selected_item_line;
        Patapon* patapon;
        Patapon* patapon2;
        Patapon* patapon3;

        sf::Texture t_background;
        PSprite s_background;
        float ratioX;
        float ratioY;

        float pataponY;
        float floorY;

        bool obelisk = false;

        bool missionStarted = false;

        sf::Font f_font;
        PText t_title;
        int currentItemPosition;
        vector<bool> enabledPositons;
        /// Barracks text
        PText t_unit_name;
        PText t_unit_status_text;

        PText t_unit_level;
        PText t_unit_experience;
        PText t_unit_damage;

        PText t_unit_hp;
        PText t_unit_crit;
        PText t_unit_attack_speed;


        /// Barracks stat text
        PText s_unit_level;
        PText s_unit_experience;
        PText s_unit_damage;

        PText s_unit_hp;
        PText s_unit_crit;
        PText s_unit_attack_speed;

        /// Barracks Equipment Text
        PText t_unit_rarepon_name;
        PText t_weapon_name;
        PText t_weapon2_name;
        PText t_armour_name;
        PText t_mask_name;


        /// menu background
        PSprite s_menu_bkg;

        /// highlighted unit
        PSprite s_pon_highlight;

        /// class icon
        PSprite s_class_icon;

        /// unit icon
        PSprite s_unit_icon;

        /// weapon icon
        PSprite s_weapon_icon;

        /// weapon2 icon
        PSprite s_weapon2_icon;

        /// armour icon
        PSprite s_armour_icon;

        /// mask icon
        PSprite s_mask_icon;

        int current_selected_pon=0;

        bool MenuMode = false;


        /// mask icon
        PSprite mask_icon;

        /// spear icon
        PSprite spear_icon;

        /// misc icon
        PSprite misc_icon;

        /// armour icon
        PSprite armour_icon;
        int currentItemId;

        int activeCategory=1;
        int activeSubcategory=0;

        int numItemRows;
        int numItemColumns = 4;
        int currentRow = 0;

        int inventoryGridXPos = 0;
        int inventoryGridYPos = 0;
        sf::RectangleShape mm_inventory_background;
        Camera camera;
        Menu *parentMenu;
        int currentMenuPosition;
        std::vector<float> possibleMenuPositions;
        void Initialise(Config *thisConfig, std::map<int,bool> *keymap,V4Core *parent,Menu *curParentMenu);
        void Update(sf::RenderWindow &window, float fps);
        void EventFired(sf::Event event);
        void SetTitle(int menuPosition);
        sf::RectangleShape mm_description_background;
        sf::RectangleShape mm_highlighted_tile;

        int current_pon=0;

        PText t_itemtitle;
        std::vector<PText> t_itemdescription;

        void OnExit();
        void OpenBarracksMenu();
        void RefreshStats();
        void UpdateButtons();
        Barracks();
        ~Barracks();



};


#endif // BARRACKSMENU_H
