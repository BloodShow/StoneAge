#include "version.h"
#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>


#include "configfile.h"
#include "util.h"
#include "net.h"
#include "msignal.h"
#include "buf.h"
#include "object.h"
#include "char.h"
#include "char_data.h"
#include "item.h"
#include "readmap.h"
#include "function.h"
#include "saacproto_cli.h"
#include "lssproto_serv.h"
#include "readnpc.h"
#include "log.h"
#include "handletime.h"
#include "title.h"
#include "encount.h"
#include "enemy.h"
#include "battle.h"
#include "magic_base.h"
#include "pet_skill.h"
#include "item_gen.h"
#include "petmail.h"
#include "npc_quiz.h"


#ifdef _M_SERVER
#include "mclient.h"
#endif

#ifdef _NPCSERVER_NEW
#include "npcserver.h"
#endif

#ifdef _PROFESSION_SKILL			// WON ADD ����ְҵ����
#include "profession_skill.h"
#endif

#ifdef _ITEM_QUITPARTY
#include "init.h"
//int itemquitparty_num = 0;
//static DisappearItem *Disappear_Item;
#endif

#define MESSAGEFILE "hoge.txt"

#define OPTIONSTRING "d:f:h"
#define usage() print( "Usage: %s ["OPTIONSTRING"]\n", getProgname() );

void printUsage( void )
{
    usage();
  /*print( "Usage: %s ["OPTIONSTRING"]\n", progname );*/
    print( "          [-d debuglevel]        default value is 0\n" );
    print( "          [-f configfilename]    default value is setup.cf\n"
        );
}

/*
 * ��Ѩ������̼�ë��  ����
 *
 * ¦��
 *      argc      ٯ  ����
 *      argv      ٯ  ��    
 * ߯Ի��
 *      TRUE(1)     ����ئ��Ѩ������̼�¦�ѷ�������
 *      FALSE(0)    ����ئ��Ѩ������̼�¦�ѷ�������
 */
BOOL parseCommandLine( int argc , char** argv )
{
    int c;                          /* getopt ƥ���� */
    extern char* optarg;            /* getopt ƥ���� */


    while( ( c = getopt( argc, argv ,OPTIONSTRING )) != -1 ){
        switch( c ){
        case 'd':
        {
            int     debuglevel;
            if( !strtolchecknum( optarg, (int*)&debuglevel, 10,
                                 INT)){
                print( "Specify digit number\n" );
                return FALSE;
            }
            setDebuglevel( debuglevel );
            break;
        }
        case 'f':
            setConfigfilename( optarg );
            break;
        case 'h':
            printUsage();
            return FALSE;
            break;

        default:
            printUsage();
            return FALSE;
            break;

        }
    }
    return TRUE;
}


/*
 * ����  ��ë�ɡ�������
 * ¦��
 *      env     ����  �Ѽ�    
 *
 * �ἰ��������ئ��
 */
BOOL parseEnvironment( char** env )
{
    if( getDebuglevel() >= 3 ){
        int index=0;
        while( env[index] != NULL )print( "%s " , env[index++] );
        print( "\n" );
    }
    return TRUE;
}




/*lsgen�ü�����ƽ�����������뼰  ���*/
#define LSGENWORKINGBUFFER  65536*4



#define GOTORETURNFALSEIFFALSE(x) if(!(x))goto RETURNFALSE
/*
 * �����������
 * ¦��
 *      argc    argv����
 *      argv    ��Ѩ������̼�¦��
 *      env     ����  ��
 * ߯Ի��
 *      TRUE(1) ��  
 *      FALSE(1) ��  
 */
BOOL init(int argc , char** argv , char** env )
{
#ifdef _ITEM_QUITPARTY
    FILE *f;
	int i;
	char line[256];
#endif
    srand( getpid());
    print( "This Program is compiled at %s %s by gcc %s\n",
           __DATE__ , __TIME__ , __VERSION__ );

    defaultConfig( argv[0] );
    signalset();

    GOTORETURNFALSEIFFALSE(parseCommandLine( argc , argv ));
    GOTORETURNFALSEIFFALSE(parseEnvironment( env ));

    {
        Char    aho;
        debug( sizeof( aho ), d);
        debug( sizeof( aho.data ), d);
        debug( sizeof( aho.string ), d);
        debug( sizeof( aho.flg ),d);
        debug( sizeof( aho.indexOfExistItems ), d);
        debug( sizeof( aho.haveSkill ), d);
        debug( sizeof( aho.indexOfHaveTitle ), d);
        debug( sizeof( aho.addressBook ),d);
        debug( sizeof( aho.workint ),d);
        debug( sizeof( aho.workchar ),d);
    }

    print( "�����ļ�: %s\n" , getConfigfilename() );

    GOTORETURNFALSEIFFALSE(readconfigfile( getConfigfilename() ) );
    //ttom start
    {  int iWork = setEncodeKey();
       if( iWork == 0 ){
       // �޼������ƽ��ëɬ��
       printf( "----------------------------------------\n" );
       printf( "-------------[����] �޷����� %s\n", getConfigfilename() );
       printf( "----------------------------------------\n" );
       exit( 1 );
       }else{
            // �޼������ƽ��ëɬ��
               printf( "���� = %d\n", iWork );
       }
    }
    // AcWBuffëɬ��
    {   int iWork = setAcWBSize();
        if( iWork == 0 ){
           printf( "----------------------------------------\n" );
           printf( "-------------[AC����] �޷����� %s\n", getConfigfilename() );
           printf( "----------------------------------------\n" );
           exit( 1 );
           }else{
                   printf( "AC���� = %d\n", iWork );
           }
    }
    //ttom end

    if( getDebuglevel() >= 1 ){
//		print("ServerType: %d\n", getServerType() );
        print("���Եȼ�: %d\n" , getDebuglevel() );
        print("�ڴ浥Ԫ: %d\n"  , getMemoryunit() );
        print("�ڴ浥Ԫ����: %d\n" , getMemoryunitnum() );

        print("�˺ŷ�������ַ: %s\n" , getAccountservername() );
        print("�˺ŷ������˿�: %d\n" , getAccountserverport() );
        print("��½����������: %s\n",
              getGameservername());
        print("��½����������: %s\n", getAccountserverpasswd());

        print("�ȴ����Ӷ˿�: %d\n",  getPortnumber() );

        print("��������к�: %d\n",  getServernumber() );

        print("�ظ���ַʹ��: %d\n",  getReuseaddr() );


        print("�����������: %d\n",
              getFdnum() );
        print("������߳���: %d\n" , getPetcharnum() );
        print("���������Ŀ: %d\n"  , getOtherscharnum() );
        print("��������Ŀ: %d\n",  getObjnum() );
        print("�����Ʒ��Ŀ: %d\n",  getItemnum() );
        print("���ս����Ŀ: %d\n",  getBattlenum() );
#ifdef _GET_BATTLE_EXP
				print("ս�����鱶��: %d\n",  getBattleexp() );
#endif
        print("�����ļ�Ŀ¼: %s\n"  , getTopdir());
        print("��ͼ�ļ�Ŀ¼: %s\n"  , getMapdir());
        print("��ͼ��ʶ�ļ�: %s\n"  , getMaptilefile());
        print("��Ʒ�����ļ�: %s\n"  , getItemfile());
        print("����ս���ļ�: %s\n"  , getInvfile());
        print("��ʾλ���ļ�: %s\n"  , getAppearfile());
        print("���������ļ�: %s\n"  , getEffectfile());
        print("ͷ�������ļ�: %s\n"  , getTitleNamefile());
        print("ͷ�������ļ�: %s\n"  , getTitleConfigfile());
        print("���������ļ�: %s\n"  , getEncountfile());
        print("������Ⱥ�ļ�: %s\n"  , getGroupfile());
        print("��������ļ�: %s\n"  , getEnemyBasefile());
        print("���������ļ�: %s\n"  , getEnemyfile());
        print("����ħ���ļ�: %s\n"  , getMagicfile());

#ifdef __ATTACK_MAGIC
        print("����ħ���ļ�: %s\n" , getAttMagicfileName() );
#endif

        print("���＼���ļ�: %s\n"  , getPetskillfile());

#ifdef _PROFESSION_SKILL			// WON ADD ����ְҵ����
        print("ְҵ�����ļ�: %s\n"  , getProfession());
#endif

        print("��Ʒ�ɷ��ļ�: %s\n", getItematomfile());
        print("���������ļ�: %s\n", getQuizfile());
#ifdef _BLACK_MARKET
				print("�����г��ļ�: %s\n", getBMItemFile());
#endif
#ifdef _GMRELOAD
				print("G M �����ļ�: %s\n", getGMSetfile());
#endif
        print("��־��¼�ļ�: %s\n"  ,  getLsgenlogfilename() );
        print("��ԭ����Ŀ¼: %s\n"  , getStoredir());
        print("NPC ����Ŀ¼: %s\n"  , getNpcdir());
        print("��־�����ļ�: %s\n",  getLogdir());
        print("��־�����ļ�: %s\n", getLogconffile() );
        print("GM��ָ������: %s\n", getChatMagicPasswd() );
        print("ʹ��GM��Ȩ��: %d\n", getChatMagicCDKeyCheck() );

        print("NPC ģ����Ŀ: %d\n"  , getNpctemplatenum() );
        print("NPC �����Ŀ: %d\n"  , getNpccreatenum() );

        print("��·ʱ����: %d\n"  , getWalksendinterval());
        print("������м��: %d\n"  , getCAsendinterval_ms());
        print("���Ŀ����: %d\n"  , getCDsendinterval_ms());
        print("ִ��һ��ʱ��: %d\n"  , getOnelooptime_ms());
        print("�������ʱ��: %d\n"  , getPetdeletetime());
        print("�������ʱ��: %d\n"  , getItemdeletetime());
#ifdef _DEL_DROP_GOLD
				print("ʯ�����ʱ��: %d\n"  , getGolddeletetime());
#endif
        print("���ݱ�����: %d\n"  , getCharSavesendinterval());

        print("��Ƭ�����Ŀ: %d\n",getAddressbookoffmsgnum());
        print("��ȡƵ��Э��: %d\n",getProtocolreadfrequency());

        print("���Ӵ�������: %d\n",getAllowerrornum());

#ifdef	_M_SERVER
		    print("GM��������ַ: %s\n",getmservername());
		    print("GM�������˿�: %d\n",getmserverport());
#endif
#ifdef _NPCSERVER_NEW
				print("Npc��������ַ: %s\n",getnpcserveraddr());
				print("Npc�������˿�: %d\n",getnpcserverport());
#endif
#ifdef _NEW_PLAYER_CF
				print("��������ת��: %d\n",getNewplayertrans());
				print("��������ȼ�: %d\n",getNewplayerlv());
				print("���������Ǯ: %d\n",getNewplayergivegold());
				print("��������ȼ�: %d\n",getNewplayerpetlv());
				print("����ӵ�г���: NO1:%d NO2:%d NO3:%d NO4:%d NO5:%d\n",getNewplayergivepet(0),
																																	getNewplayergivepet(1),
																																	getNewplayergivepet(2),
																																	getNewplayergivepet(3),
																																	getNewplayergivepet(4));
				print("����ӵ����Ʒ: ITEM1:%d ITEM2:%d ITEM3:%d ITEM4:%d ITEM5:%d\n"
							"��������������ITEM1:%d ITEM2:%d ITEM3:%d ITEM4:%d ITEM5:%d\n"
							"��������������ITEM1:%d ITEM2:%d ITEM3:%d ITEM4:%d ITEM5:%d\n"
																																	,getNewplayergiveitem(0)
																																	,getNewplayergiveitem(1)
																																	,getNewplayergiveitem(2)
																																	,getNewplayergiveitem(3)
																																	,getNewplayergiveitem(4)
																																	,getNewplayergiveitem(5)
																																	,getNewplayergiveitem(6)
																																	,getNewplayergiveitem(7)
																																	,getNewplayergiveitem(8)
																																	,getNewplayergiveitem(9)
																																	,getNewplayergiveitem(10)
																																	,getNewplayergiveitem(11)
																																	,getNewplayergiveitem(12)
																																	,getNewplayergiveitem(13)
																																	,getNewplayergiveitem(14));
#endif
#ifdef _UNLAW_WARP_FLOOR
		print("��ֹ���͵�ͼ: FLOOR1:%d FLOOR2:%d FLOOR3:%d FLOOR4:%d FLOOR5:%d\n",getUnlawwarpfloor(0),
																																	getUnlawwarpfloor(1),
																																	getUnlawwarpfloor(2),
																																	getUnlawwarpfloor(3),
																																	getUnlawwarpfloor(4));
#endif
#ifdef _UNREG_NEMA
		print("��ֹ��������: NAME1:%s NAME2:%s NAME3:%s NAME4:%s NAME5:%s\n",getUnregname(0),
																																	getUnregname(1),
																																	getUnregname(2),
																																	getUnregname(3),
																																	getUnregname(4));
#endif
#ifdef _TRANS_LEVEL_CF
		print("����ȼ�ת��: %d\n",getChartrans());
		print("����ȼ�ת��: %d\n",getPettrans());
#endif

#ifdef _LOCK_IP
		print("���� IP ����: %s\n",getLockipPath());
#endif
    }

	{	//andy_add 2003/05/05 check GameServer Name
		char *GameServerName;
		GameServerName = getGameserverID();
		if( GameServerName == NULL || strlen( GameServerName) <= 0 )
			return FALSE;
		print("\n��Ϸ������ID: %s\n",  GameServerName );
	}
	
    print("��ʼ��ʼ��\n" );

//#define DEBUG1( arg... ) if( getDebuglevel()>1 ){##arg}
    print( "�����ڴ�ռ�..." );
    GOTORETURNFALSEIFFALSE(configmem( getMemoryunit(),
                                      getMemoryunitnum() ) );
    GOTORETURNFALSEIFFALSE(memInit());
	print( "���\n" );
    if( !initConnect(getFdnum()) )
        goto MEMEND;
    while( 1 ){
        print( "���԰󶨱��ض˿� %d... " , getPortnumber());
        bindedfd = bindlocalhost( getPortnumber() );
        if( bindedfd == -1 )
            sleep( 10 );
        else
            break;

    }
	print( "���\n" );
	print( "��������..." );
    if( !initObjectArray( getObjnum()) )
        goto CLOSEBIND;
	print( "���\n" );
	print( "��������..." );
    if(!CHAR_initCharArray( getFdnum(), getPetcharnum(),getOtherscharnum()) )
        goto CLOSEBIND;
	print( "���\n" );
	print( "������Ʒ:%s...", getItemfile());
    if(!ITEM_readItemConfFile( getItemfile()) )
        goto CLOSEBIND;
    if(!ITEM_initExistItemsArray( getItemnum() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "����ս��..." );
    if(!BATTLE_initBattleArray( getBattlenum() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��������ģ��..." );
    if( !initFunctionTable() )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ʼ���ʼ�..." );
    if( !PETMAIL_initOffmsgBuffer( getAddressbookoffmsgnum() ))
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡ����ս���ļ�..." );
    if( !CHAR_initInvinciblePlace( getInvfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡ��ʾλ���ļ�..." );
    if( !CHAR_initAppearPosition( getAppearfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡͷ�������ļ�..." );
    if( !TITLE_initTitleName( getTitleNamefile() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡͷ�������ļ�..." );
    if( !TITLE_initTitleConfig( getTitleConfigfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡ���������ļ�..." );
    if( !ENCOUNT_initEncount( getEncountfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡ��������ļ�..." );
    if( !ENEMYTEMP_initEnemy( getEnemyBasefile() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡ���������ļ�..." );
    if( !ENEMY_initEnemy( getEnemyfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

	print( "��ȡ������Ⱥ�ļ�..." );
    if( !GROUP_initGroup( getGroupfile() ) )
        goto CLOSEBIND;
	print( "���\n" );
	print( "��ȡħ���ļ�..." );
    if( !MAGIC_initMagic( getMagicfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

		#ifdef __ATTACK_MAGIC

	print( "��ȡħ�������ļ�..." );

    if( !ATTMAGIC_initMagic( getAttMagicfileName() ) )
//		if( !ATTMAGIC_initMagic( getMagicfile() ) )
        goto CLOSEBIND;

	print( "ħ�������ļ� -->%s..." , getAttMagicfileName());
	print( "���\n" );

    #endif
 
	print( "��ȡ���＼���ļ�..." );
    if( !PETSKILL_initPetskill( getPetskillfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

#ifdef _PROFESSION_SKILL			// WON ADD ����ְҵ����
	print( "��ȡְҵ�����ļ�..." );
	if( !PROFESSION_initSkill( getProfession() ) ){
		goto CLOSEBIND;
	}
	print( "���\n" );
#endif

    /* ʧ��  ة����    ë  �� */
	print( "��ȡ��Ʒ�ɷ��ļ�..." );
    if( !ITEM_initItemAtom( getItematomfile()) )
        goto CLOSEBIND;
	print("���\n" );

	print( "��ʼ����Ʒ���ٻ���..." );
    if( !ITEM_initItemIngCache() )
        goto CLOSEBIND;
	print("���\n" );
    
	print( "��ʼ����Ʒ����..." );
    if( !ITEM_initRandTable() )
        goto CLOSEBIND;
	print("���\n" );
    
	print( "��ȡ���������ļ�..." );
    if( !CHAR_initEffectSetting( getEffectfile() ) )
        goto CLOSEBIND;
	print( "���\n" );
	print( "��ȡ���������ļ�..." );
    if( !QUIZ_initQuiz( getQuizfile() ) )
        goto CLOSEBIND;
	print( "���\n" );

#ifdef _BLACK_MARKET
	print( "�����г��ļ�..." );
	if ( !LoadBMItem(getBMItemFile()) )
		goto CLOSEBIND;
	print( "���\n" );
#endif

#ifdef _GMRELOAD
	print( "��ȡGM�����ļ�..." );
	if ( !LoadGMSet( getGMSetfile() ) )
		goto CLOSEBIND;
	print( "���\n" );
#endif

#ifdef _GMRELOAD
	print( "��ȡ���������ļ�..." );
	if ( !LoadEXP( getEXPfile() ) )
		goto CLOSEBIND;
	print("��ߵȼ�: %d...",getMaxLevel());
	print("һ��ȼ�: %d...",getYBLevel());
	print( "���\n" );
#endif

#ifdef _ANGEL_SUMMON
	print("��ȡ�����б��ļ�...");
	if( !LoadMissionList( ) )
		goto CLOSEBIND;
	print("���\n");
#endif

#ifdef _CONTRACT
	print("��ȡ��Լ�ļ�...");
	//if( !LoadMissionList( ) )
	//	goto CLOSEBIND;
	if( !ITEM_initContractTable() )
		goto CLOSEBIND;
	print("���\n");
#endif

#ifdef _JOBDAILY
	print("��ȡ������־�ļ�...");
	if(!LoadJobdailyfile())
		goto CLOSEBIND;
	print("���\n");
#endif

#ifdef _RACEMAN
	print("��ȡ���������ļ�...");
	if(!LoadRacepetfile())
		goto CLOSEBIND;
	print("���\n");
#endif

#ifdef _LOCK_IP
	print("��ȡ����IP�ļ�...");
	if(!loadLockip( getLockipPath()))
		goto CLOSEBIND;
	print("���\n");
#endif

	print( "������ͼ..." );
    if( !MAP_initReadMap( getMaptilefile() , getMapdir() ))
        goto CLOSEBIND;
	print( "���\n" );
	print( "��ȡNPC�ļ�..." );
    if( !NPC_readNPCSettingFiles( getNpcdir(), getNpctemplatenum(),
                                  getNpccreatenum() ) )
        goto CLOSEBIND;
	print( "���\n" );
	print( "��ʼ�� NPC ������... " );
    if( lssproto_InitServer( lsrpcClientWriteFunc, LSGENWORKINGBUFFER ) < 0 )
        goto CLOSEBIND;
	print( "���\n" );
	print( "���������˺ŷ�����... " );
    acfd = connectHost( getAccountservername(), getAccountserverport());
    if(acfd == -1)
        goto CLOSEBIND;

/*
	{
		int errorcode;
		int errorcodelen;
		int qs;

		errorcodelen = sizeof(errorcode);
		qs = getsockopt( acfd, SOL_SOCKET, SO_RCVBUF , &errorcode, &errorcodelen);
		//andy_log
		print("\n\n GETSOCKOPT SO_RCVBUF: [ %d, %d, %d] \n", qs, errorcode, errorcodelen);
	}
*/

	print( "���\n" );
    initConnectOne( acfd, NULL , 0 );
    if( !CONNECT_acfdInitRB( acfd)) goto CLOSEAC;
    if( !CONNECT_acfdInitWB( acfd)) goto CLOSEAC;
    CONNECT_setCtype( acfd, AC );
	
	print( "��ʼ�� NPC �ͻ��� ... " );
    /*  rpc(client)������� */
    if( saacproto_InitClient( lsrpcClientWriteFunc,LSGENWORKINGBUFFER, acfd) < 0 )
        goto CLOSEAC;
	print( "���\n" );

	print( "���˺ŷ��������͵�½����... " );
    /*  ����̼�ۢ��ë����  */
    saacproto_ACServerLogin_send(acfd, getGameservername(),
                                 getAccountserverpasswd());
	print( "���\n" );

    if( isExistFile( getLsgenlogfilename() ) ){
        lssproto_SetServerLogFiles( getLsgenlogfilename(),
                                    getLsgenlogfilename() );
        saacproto_SetClientLogFiles( getLsgenlogfilename(),
                                     getLsgenlogfilename() );
    }


	print( "��ʼ�������\n" );

#ifdef _MUSEUM
	if( getMuseum() )
		print("\n����ʯ�������!!\n");
	else
		print("\n������ͨ����!!\n");
#endif

	print( "��ʼ��ʼ��־\n" );
    {
        char    logconffile[512];
        snprintf( logconffile, sizeof( logconffile), "%s/%s" ,
                  getLogdir(), getLogconffile() );
        if( !initLog( logconffile ) )
            goto CLOSEAC;
    }

#ifdef _M_SERVER
	print( "��������GM������... " );
    mfd = connectmServer( getmservername(), getmserverport());
	if (mfd ==-1 ){
		print( "����GM������ʧ��... " );
	}else{
		initConnectOne( mfd, NULL , 0 );
		print( "�ɰ�����GM������... " );
	}
#endif

#ifdef _NPCSERVER_NEW
	npcfd = connectNpcServer( getnpcserveraddr(), getnpcserverport());
	if( npcfd == -1 ){
		print( "����NPC������ʧ��... " );
	}else{
		initConnectOne( npcfd, NULL , 0 );
		print( "�ɰ�����NPC������... " );
		NPCS_NpcSLogin_send( npcfd);
	}
#endif
	
#ifdef _ITEM_QUITPARTY
	print( "��ȡ�����ɢ��Ʒ��ʧ�ļ�..." );
    
	//��ȡ����
    f = fopen( getitemquitparty(), "r" );
	if( f != NULL ){
		while( fgets( line, sizeof( line ), f ) ){
			if( line[0] == '#' )continue;
            if( line[0] == '\n' )continue;
		    chomp( line );
			itemquitparty_num++;
		}
		if( fseek( f, 0, SEEK_SET ) == -1 ){
			print( "��Ʒ¼�Ҵ���\n" );
			fclose(f);
			goto CLOSEAC;
		}
		//�������
		Disappear_Item = allocateMemory( sizeof(struct tagDisappearItem) * itemquitparty_num );
		if( Disappear_Item == NULL ){
			print( "�޷������ڴ� %d\n", sizeof(struct tagDisappearItem) * itemquitparty_num );
			fclose( f );
			goto CLOSEAC;
		}

		i = 0;
		//�����߱�Ŵ��� Disappear_Item.string
		while( fgets( line, sizeof( line ), f ) ){
			if( line[0] == '#' )continue;
			if( line[0] == '\n' )continue; 
			chomp( line );
			sprintf( Disappear_Item[i].string,"%s",line );
			print("\n���߱��:%s", Disappear_Item[i].string );
			i++;
		}
		fclose(f);
	}
#endif

    DEBUG_ADJUSTTIME = 0;
    print( "\n" );
    return TRUE;

CLOSEAC:
    close( acfd );
CLOSEBIND:
    close( bindedfd );
    endConnect();
MEMEND:
    memEnd();
RETURNFALSE:
    return FALSE;
}
