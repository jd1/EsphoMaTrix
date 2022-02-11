#include "esphome.h"

namespace esphome
{   
    EHMTX_store::EHMTX_store(EHMTX *config)
    {
        for (uint8_t i = 0; i < MAXQUEUE; i++)
        {
            this->slots[i] = new EHMTX_screen(config);
        }
        this->active_slot = 0;
    }

    EHMTX_screen* EHMTX_store::find_free_screen(uint8_t icon)
    {
        ESP_LOGD(TAG,"findfreeslot for icon: %d",icon);
        for (uint8_t i = 0; i < MAXQUEUE; i++)
        {
            EHMTX_screen* screen = this->slots[i];
            if (screen->icon == icon)
            {
                return screen;
            }
        }
    
        time_t ts = this->clock->now().timestamp;
        for (uint8_t i = 0; i < MAXQUEUE; i++)
        {
            EHMTX_screen* screen = this->slots[i];
            if (screen->endtime <= ts)
            {
                return screen;
            }
        }
        //Default to first screen. Is this intended?
        return this->slots[0];
    }

    void EHMTX_store::delete_screen(uint8_t icon)
    {
        for (uint8_t i = 0; i < MAXQUEUE; i++)
        {
            this->slots[i]->del_slot(icon);
        }
    }

    std::optional<uint8_t> find_slot_of_active_screens(uint8_t start, uint8_t end)
    {
        for (uint8_t slot = start; slot < end; slot++)
        {
            EHMTX_screen* screen = this->slots[slot];
            if (screen->active())
            {
                return slot;
            }
        }
        return {}; 
    }

    bool EHMTX_store::move_next()
    {
        if (this->count_active_screens()== 1)
        {
            //Find first and only active screen
            std::optional slot = this->find_slot_of_active_screens(0, MAXQUEUE);
            if(slot) {
                this->active_slot = slot;
                return true;
            }
        }

        //Find active screen between active slot and end of array
        std::optional slot = this->find_slot_of_active_screens(this->active_slot + 1, MAXQUEUE);
        if(slot) {
            this->active_slot = slot;
            return true;
        }
        
        //Find active screen between 0 and active slot
        std::optional slot = this->find_slot_of_active_screens(0, this->active_slot);
        if(slot) {
            this->active_slot = slot;
            return true;
        }

        //No active screen found
        this->active_slot = nullptr;
        return false;
    }

    EHMTX_screen* EHMTX_store::current()
    {
        return this->slots[this->active_slot];
    }

    uint8_t EHMTX_store::count_active_screens()
    {
        uint8_t count = 0;
        for (uint8_t screen = 0; screen < MAXQUEUE; screen++) {
            if (this->slots[screen]->active()) {
                count++;
            }
        }
        return count;
    }

    void EHMTX_store::log_status()
    {
        uint8_t status = 0;
        time_t ts = this->clock->now().timestamp;
        ESP_LOGI(TAG, "status active slot: %d", status, this->active_slot);
        ESP_LOGI(TAG, "status screen count: %d of %d", this->count_active_screens(), MAXQUEUE);
        for (uint8_t i = 0; i < MAXQUEUE; i++)
        {
            if (this->slots[i]->active())
            {
                EHMTX_screen* screen = this->slots[i];
                int td = screen->endtime - ts;
                ESP_LOGI(TAG, "status slot %d icon %d text: %s end: %d sec", i, screen->icon, screen->text.c_str(), td);
            }
        }
  }
}
