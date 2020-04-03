/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         schedule.h                      **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/

#ifdef WIN32
#define SCALL __cdecl
#else
#define SCALL
#endif

#define MAX_EVENT   5
#define EVENT_PSG   0
#define EVENT_VOICE 1
#define EVENT_KEYIN 2
#define EVENT_DISK  3

// イベントスタイルフラグ
// bit0: 繰り返し指示
#define	EV_ONETIME	(0x00)	/* ワンタイム */
#define	EV_LOOP		(0x01)	/* ループ */
// bit1,2: 周期指定単位
#define	EV_HZ		(0x00)	/* Hz */
#define	EV_US		(0x02)	/* us */
#define	EV_MS		(0x04)	/* ms */
#define	EV_STATE	(0x08)	/* CPUステート数 */

typedef struct {
		int Active;			// 有効・無効
		int Period;			// 1周期のクロック数
		int Clock;			// 残りクロック数
		double nps;			// イベント発生周波数(Hz)
		
		int ncount;			// イベント発生回数
		int ratio;			// イベント発生率
		int (SCALL *CallbackProc)( void*);	// コールバック関数
	} evinfo;


void Event_init(void);
int Event_Add( int id, double hz ,int flag, int (SCALL *CallbackProc)( void*) );
void Event_Del(int id);
int Event_Update(int clk);
double Event_Scale( int id );
void Event_Reset( int id );
int Event_isActive(int id);

