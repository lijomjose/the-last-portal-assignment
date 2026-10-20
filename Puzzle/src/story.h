#ifndef PORTAL_STORY_H
#define PORTAL_STORY_H

/* Story copy is separate from the student operations. Keep the playable goal
 * visible in a stationary card even while the opening text scrolls away. */
typedef struct {
    const char *chapter, *title, *image, *narrative, *mission, *reference_mission, *data;
} RoomStory;

static const RoomStory room_stories[3] = {
    {"CHAPTER I", "A MEMORY OF A MISTAKE", "assets/story/memory.png",
     "Beyond the last door, Caitlyn finds a station slowly falling silent.\n\n"
     "One careless push has trapped a power crate against the wall. She cannot pull it free. The station should remember the board before that push, but its memory routines are missing.\n\n"
     "Restore the stack. Save a whole board before each action. When time is rewound, recover the newest saved board first.\n\n"
     "Then guide the crate onto the pressure plate. A remembered mistake can open the next door.",
     "Restore history_push / history_pop. Press Z, push the crate onto the green plate, then walk into EXIT.",
     "The reference code already works. Press Z, push the crate onto the green plate, then walk into EXIT.",
     "STACK: complete BoardState snapshots, saved before actions. F1 explains the room and its code."},
    {"CHAPTER II", "A ROUTE TO REMEMBER", "assets/story/courier.png",
     "Caitlyn reaches the security bay. Her courier carries the power cell that can open the final door.\n\n"
     "A random safe route lights up across the floor, but the failing station can hold its lights for only 30 seconds. Hide the route early when you are ready, or let the countdown finish. When the lights go dark, every tile looks the same.\n\n"
     "Memorize the arrows from Start to Dock. Then record the ENTIRE route in a queue: every new direction joins the rear; the courier follows the oldest command at the front. There is room for the whole plan.\n\n"
     "Launch only when every arrow is entered. One wrong step triggers the security laser and destroys the courier. A new attempt brings a new route. One perfect run brings Caitlyn closer to Home.",
     "Restore enqueue/dequeue. Space reveals for 30 seconds; Space again hides it early. Then queue ALL arrows; Space launches. Backspace edits; C clears; R retries. One wrong step destroys the bot.",
     "Space reveals for 30 seconds; Space again hides it early. Memorize, then queue ALL arrows and Space launches. Backspace edits; C clears; R retries. Deliver safely, then walk right to EXIT.",
     "QUEUE: the whole remembered plan, FRONT first. Capacity 64, routes 10-14 moves. No single-step probing or rewind. F1 explains."},
    {"CHAPTER III", "CLASH OF QUEUE AND STACK", "assets/story/portal.png",
     "The courier restores the station's power. Caitlyn reaches a narrow alley. Home glows at its far end.\n\n"
     "The Hollow Warden stands in the only path. It launches whole sequences of opening brackets, each read from left to right. Every opening is pushed onto an enemy stack.\n\n"
     "Caitlyn types a whole sequence of closing brackets, also from left to right. Her counters enter a queue. The enemy's newest opening must meet her oldest response: stack TOP against queue FRONT.\n\n"
     "An attack of ([ needs ]). Launch with Enter. Clear all three sequences, then unleash HOMEBOUND and walk through the portal.",
     "Restore attack insertion/removal and diagnose the shield and charge faults. Space begins; 1/2/3 types counters; Enter launches. After three sequences, F finishes; Right goes Home.",
     "Space begins. Type whole counters with 1/2/3, then Enter launches. Enemy ([ needs ]). After three sequences, F finishes; Right goes Home.",
     "STACK: enemy opening brackets, head = TOP. QUEUE: your closing brackets, oldest = FRONT. Both read left to right. F1 explains."}
};

#endif
