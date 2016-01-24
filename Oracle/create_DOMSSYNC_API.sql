-- ---------------------------------------------------------------------------------------------------
--
-- The procedures in this package will be called from the the DOMSSync web service
-- for several purposes:
--
-- 1) Some procedures obtain basic abnormal state information for the overall network.
-- DOMSSync will use this information to compare to basic abnormal states in DMS, 
-- and display the differences to an operator.  The request can be filtered by 
-- substation and by object type.
--
-- 2) Some procedures obtain basic state information on a list of objects.  This is
-- used by DOMSSync to refresh the state of particular item(s) in the list of differences 
-- displayed to the operator.
--
-- 3) Some procedures obtain details on an object.  DOMSSync will request
-- this information right after the operator requests that an operation be applied to 
-- synchronize the networks.  Once DOMSSync receives this information, it can request
-- that an operation be applied using the standard interface methods that already exist 
-- between DMS and OMS.
--
-- We use associative arrays as input.
-- We use global temporary tables to store the data from the incoming associative array.
-- We have to loop through the array and copy into our temp tables.
-- It's not an efficient approach but I haven't found a better alternative.  Refer to:
--	  http://stackoverflow.com/questions/1715978/how-to-use-an-oracle-associative-array-in-a-sql-query
--
-- INSTALLATION:
--
--		=======================================================================
--			NOTE: ALWAYS INSTALL THIS PACKAGE UNDER THE OMS [ASTATUS1] USER!
--			Permissions and client code have been written with this expectation.
--		=======================================================================
--
--		connect astatus1@bsctst01
--		@"[appdevtfs path]\Dist-DMS\OMS-DMS\Oracle\create_DOMSSYNC_API.sql"
--
-- ---------------------------------------------------------------------------------------------------
--
-- The global tables are created once and then persist.
-- Here we are creating them but they probably already exist.
-- NOTE: Drop doesn't always work if the table exists ("table already in use")
-- This seems to happen if you are (eg) working with the table in a SQL*Plus session.
drop TABLE ASTATUS1.DOMSSYNC_SUBSTATIONS;
drop TABLE ASTATUS1.DOMSSYNC_OBJTYPES;
drop TABLE ASTATUS1.DOMSSYNC_OBJIDS;
CREATE GLOBAL TEMPORARY TABLE ASTATUS1.DOMSSYNC_SUBSTATIONS (
  SUB_ID  VARCHAR2(10)
) ON COMMIT PRESERVE ROWS;
CREATE GLOBAL TEMPORARY TABLE ASTATUS1.DOMSSYNC_OBJTYPES (
  OBJ_TYPE  NUMBER
) ON COMMIT PRESERVE ROWS;
CREATE GLOBAL TEMPORARY TABLE ASTATUS1.DOMSSYNC_OBJIDS (
  OBJ_ID  VARCHAR2(40),
  OBJ_TYPE  NUMBER
) ON COMMIT PRESERVE ROWS;
--
create or replace PACKAGE ASTATUS1.DOMSSYNC_API AS
	-- associative arrays
	-- these are in-memory arrays with data provided by the caller
	-- they can't be used directly in SQL (DOH! stupid!)
	-- we'll copy the data into our temporary tables
	TYPE SUBSTATION_TABLE is table of VARCHAR2(15) index by BINARY_INTEGER;
	TYPE OBJTYPE_TABLE is table of INTEGER index by BINARY_INTEGER;
	TYPE OBJID_TABLE is table of VARCHAR2(40) index by BINARY_INTEGER;
	--
	-- output ref cursor
	-- We define the type of record returned in the ref cursor.
	-- This makes it more strongly typed, and makes it easier to call from PL/SQL.
	TYPE STATE_RESULT_RECORD IS RECORD
	(
		DateTime DATE,
		ObjectID VARCHAR2(40),
		ObjectType NUMBER(4),
		Substation VARCHAR2(15),		-- NOTE: even tho we use the short sub id for input, we return the long sub id
		Phases NUMBER(4),
		NormalState NUMBER(4),
		-- opstate-specific
		Bypass NUMBER(4),
		-- tag-specific
		Comments VARCHAR2(240),
		TagLevel NUMBER(3),
		EquipType NUMBER(4),
		EquipID VARCHAR2(40),
		TagHolder VARCHAR2(15)
	);
	-- TYPE STATE_RESULT_TABLE IS TABLE OF STATE_RESULT_RECORD;
	TYPE STATES_RESULTS_CURSOR IS REF CURSOR RETURN STATE_RESULT_RECORD;
	--
	PROCEDURE GetStates (
		IN_SUBS     IN SUBSTATION_TABLE, 
		IN_OBJTYPES IN OBJTYPE_TABLE, 
		IO_CURSOR   IN OUT STATES_RESULTS_CURSOR
    ); 
    PROCEDURE GetStatesList (
		IN_OBJIDS   IN OBJID_TABLE, 
		IN_OBJTYPES IN OBJTYPE_TABLE, 
		IO_CURSOR   IN OUT STATES_RESULTS_CURSOR
    ); 
    PROCEDURE GetLinecutDetails (
		IN_OBJID		IN VARCHAR2, 
		OUT_LINE_ID		IN OUT VARCHAR2, 
		OUT_DIST		IN OUT NUMBER
    );
    PROCEDURE GetJumperlineDetails (
		IN_OBJID		IN VARCHAR2, 
		OUT_NODE1_ID	IN OUT VARCHAR2, 
		OUT_NODE2_ID	IN OUT VARCHAR2,
		OUT_CU			IN OUT NUMBER
    );
    PROCEDURE GetTagDetails (
		IN_USERTAGID	IN VARCHAR2, 
		OUT_PLACEDBY	IN OUT VARCHAR2,
		OUT_COMPANY		IN OUT VARCHAR2,
		OUT_PHONE		IN OUT VARCHAR2,
		OUT_WORK		IN OUT VARCHAR2,
		OUT_BATTERY		IN OUT NUMBER,
		OUT_INDICATOR	IN OUT NUMBER,
		OUT_REMOVEDBY	IN OUT VARCHAR2,
		OUT_REMOVEDATE	IN OUT DATE,
		OUT_REFNODE		IN OUT VARCHAR2,
		OUT_NODEDIST	IN OUT NUMBER
    );
	PROCEDURE SetOMSAdapterIntegrated(
		IN_INTEGRATED		IN NUMBER
	);
	PROCEDURE GetOMSAdapterIntegrated(
		OUT_INTEGRATED		OUT NUMBER
	);
END DOMSSYNC_API;
/
--
-- The main API
CREATE OR REPLACE PACKAGE BODY ASTATUS1.DOMSSYNC_API AS
    --
    --
	-- The main procedure.
	-- This gets ALL 'abnormal' statuses.
	-- This is a combination of...
	--
	--		devices in abnormal state
	--		all linecuts
	--		all jumperlines
	--		all tags
	--		all large customer outages
	--
	-- These are queried separately and then all combined into one result.
	-- Note that results can be filtered by:
	--
	--		substation
	--		object type
	--
	-- Filtering is accomplished through the two input associative arrays.
	-- Results are returned via the provided in/out cursor variable.
	--
    PROCEDURE GetStates (
		IN_SUBS     IN SUBSTATION_TABLE, 
		IN_OBJTYPES IN OBJTYPE_TABLE, 
		IO_CURSOR   IN OUT STATES_RESULTS_CURSOR
    )
    IS 
		V_CURSOR STATES_RESULTS_CURSOR; 
    BEGIN 
		-- use the input variables to filter
		-- we may need to filter by substation
		-- we may need to filter by object type
		-- we may need to filter for... timestamp?  no, we don't have good timestamps atm
		--
		-- loop thru and dump array contents into global tables
		delete from DOMSSYNC_SUBSTATIONS; -- should be unnecessary, Oracle clears between sessions
		FOR i IN 1..IN_SUBS.COUNT LOOP
			insert into DOMSSYNC_SUBSTATIONS values(IN_SUBS(i));
		END LOOP;
		delete from DOMSSYNC_OBJTYPES;	  -- should be unnecessary, Oracle clears between sessions
		FOR i IN 1..IN_OBJTYPES.COUNT LOOP
			insert into DOMSSYNC_OBJTYPES values(IN_OBJTYPES(i));
		END LOOP;
   		-- query
        OPEN V_CURSOR FOR 
			with sub_map AS (
				select ds.SUB_ID as SUB_ID, s.ID as SUB_INDEX
				from DOMSSYNC_SUBSTATIONS ds LEFT JOIN SUBSTATION s 
				on ds.SUB_ID = s.CPL_SUBSTATION_NUMBER
			)
				select 
					to_date(t.timestamp, 'MM/DD/YYYY HH24:MI:SS') as DateTime, 
					cast(t.objid as varchar2(40)) as ObjectID,
					cast(t.objcat as number(4))  as ObjectType, 
					sm.SUB_ID as Substation,
					d.status as Phases,
					d.norm_state as NormalState,
					-- opstate-specific
					d.bypass_status as Bypass,
					-- tag-specific
					'' as Comments,
					0 as TagLevel,
					0 as EquipType,
					'' as EquipID,
					'' as TagHolder
				FROM temp_states t INNER JOIN device d
					on d.dfpos = t.objfpos				-- no need to check d.devcat, it's filtered below
				INNER JOIN sub_map sm
					on t.SUBSTATION_ID = sm.SUB_INDEX
				where 
					(d.status != d.norm_state or d.devcat = 26)			-- include ALL cuts
					and d.devcat in (select OBJ_TYPE from DOMSSYNC_OBJTYPES)
			UNION ALL
			(
				select 
					timestamp as DateTime,							
					cast(user_tag_id as varchar2(40)) as ObjectID,
					50 as ObjectType, 
					sm.SUB_ID as Substation,
					111 as Phases,
					0 as NormalState,
					-- opstate-specific
					0 as Bypass,
					-- tag-specific
					Comments,
					TAG_LEVEL as TagLevel,
					OBJECT_CAT as EquipType,
					OBJECT_ID as EquipID,
					TAG_HOLDER as TagHolder
				from tag_entry te INNER JOIN sub_map sm
					on te.SUBSTATION_ID = sm.SUB_INDEX
				where 
					50 in (select OBJ_TYPE from DOMSSYNC_OBJTYPES) -- filter out if needed
			UNION ALL
			(
				-- with node_map AS ()
				select 
					to_date(t.timestamp, 'MM/DD/YYYY HH24:MI:SS') as DateTime, 
					cast(t.objid as varchar2(40)) as ObjectID,
					cast(t.objcat as number(4)) as ObjectType, 
					sm.SUB_ID as Substation,
					to_number(t.curr) as Phases,
					to_number(t.normal) as NormalState,
					-- opstate-specific
					0 as Bypass,
					-- tag-specific
					'' as Comments,
					0 as TagLevel,
					0 as EquipType,
					'' as EquipID,
					'' as TagHolder
				from temp_states t INNER JOIN sub_map sm
					on t.SUBSTATION_ID = sm.SUB_INDEX
				where 
					3 in (select OBJ_TYPE from DOMSSYNC_OBJTYPES)	-- filter out if needed
					and	objcat=3 
			UNION ALL
				select 
					time_stamp as DateTime, 
					cast(dev_id as varchar2(40)) as ObjectID,
					85 as ObjectType, 
					sm.SUB_ID as Substation,
					111 as Phases,				-- can we pull this from [phases] field?
					0 as NormalState,
					-- opstate-specific
					0 as Bypass,	
					-- tag-specific
					'' as Comments,
					0 as TagLevel,
					0 as EquipType,
					'' as EquipID,
					'' as TagHolder
				from outage_repair outr INNER JOIN sub_map sm
					on outr.subs_no = sm.SUB_INDEX
				where 
					85 in (select OBJ_TYPE from DOMSSYNC_OBJTYPES)	-- filter out if needed
					and dev_cat= 'CUSTOMER' 
			));
		-- return it!
        IO_CURSOR := V_CURSOR; 
    END GetStates;
    --
    --
	-- The "list" procedure.
	-- This gets statuses for the objects in the provided list.
	-- Objects are identified by both object type and object id.
	-- Results are returned via the provided in/out cursor variable.
	--
    PROCEDURE GetStatesList (
		IN_OBJIDS   IN OBJID_TABLE, 
		IN_OBJTYPES IN OBJTYPE_TABLE, 
		IO_CURSOR   IN OUT STATES_RESULTS_CURSOR
    )
    IS 
		V_CURSOR STATES_RESULTS_CURSOR; 
    BEGIN 
		-- loop thru and dump array contents into global tables
		-- TODO: assert that both arrays are the same size; if not, throw
		delete from DOMSSYNC_OBJIDS;
		FOR i IN 1..IN_OBJIDS.COUNT LOOP
			insert into DOMSSYNC_OBJIDS values(IN_OBJIDS(i), IN_OBJTYPES(i));
		END LOOP;
   		-- query
        OPEN V_CURSOR FOR 
			select 
				to_date(t.timestamp, 'MM/DD/YYYY HH24:MI:SS') as DateTime, 
				cast(t.objid as varchar2(40)) as ObjectID,
				cast(t.objcat as number(4))  as ObjectType, 
				ss.CPL_SUBSTATION_NUMBER as Substation,
				d.status as Phases,
				d.norm_state as NormalState,
				-- opstate-specific
				d.bypass_status as Bypass,
				-- tag-specific
				'' as Comments,
				0 as TagLevel,
				0 as EquipType,
				'' as EquipID,
				'' as TagHolder
			from device d INNER JOIN temp_states t 
				on d.dfpos = t.objfpos					-- dfpos is unique, no need for devcat
			INNER JOIN SUBSTATION ss 
				on t.SUBSTATION_ID = ss.ID
			INNER JOIN DOMSSYNC_OBJIDS do
				on objid = do.OBJ_ID
				and objcat = do.OBJ_TYPE
		UNION ALL
		(
			select 
				timestamp as DateTime, 
				cast(user_tag_id as varchar2(40)) as ObjectID,
				50 as ObjectType, 
				ss.CPL_SUBSTATION_NUMBER as Substation,
				111 as Phases,
				0 as NormalState,
				-- opstate-specific
				0 as Bypass,
				-- tag-specific
				Comments,
				TAG_LEVEL as TagLevel,
				OBJECT_CAT as EquipType,
				OBJECT_ID as EquipID,
				TAG_HOLDER as TagHolder
			from tag_entry te
			INNER JOIN SUBSTATION ss 
				on te.SUBSTATION_ID = ss.ID
			INNER JOIN DOMSSYNC_OBJIDS do
				on cast(user_tag_id as varchar2(40)) = do.OBJ_ID
				and 50 = do.OBJ_TYPE
		UNION ALL
		(
			select 
				to_date(t.timestamp, 'MM/DD/YYYY HH24:MI:SS') as DateTime, 
				cast(t.objid as varchar2(40)) as ObjectID,
				cast(t.objcat as number(4)) as ObjectType, 
				ss.CPL_SUBSTATION_NUMBER as Substation,
				to_number(t.curr) as Phases,
				to_number(t.normal) as NormalState,
				-- opstate-specific
				0 as Bypass,
				-- tag-specific
				'' as Comments,
				0 as TagLevel,
				0 as EquipType,
				'' as EquipID,
				'' as TagHolder
			from temp_states t
			INNER JOIN SUBSTATION ss 
				on t.SUBSTATION_ID = ss.ID
			INNER JOIN DOMSSYNC_OBJIDS do
				on cast(t.objid as varchar2(40)) = do.OBJ_ID
				and 3 = do.OBJ_TYPE
			where t.objcat = 3			-- to prevent false positives on ID match
		UNION ALL
			select 
				time_stamp as DateTime, 
				cast(dev_id as varchar2(40)) as ObjectID,
				85 as ObjectType, 
				ss.CPL_SUBSTATION_NUMBER as Substation,
				111 as Phases,				-- can we pull this from [phases] field?
				0 as NormalState,
				-- opstate-specific
				0 as Bypass,
				-- tag-specific
				'' as Comments,
				0 as TagLevel,
				0 as EquipType,
				'' as EquipID,
				'' as TagHolder
			from outage_repair outr
			INNER JOIN SUBSTATION ss 
				on outr.subs_no = ss.ID
			INNER JOIN DOMSSYNC_OBJIDS do
				on cast(dev_id as varchar2(40)) = do.OBJ_ID
				and 85 = do.OBJ_TYPE
			where dev_cat= 'CUSTOMER'	-- to prevent false positives on ID match
		));
		-- return it!
        IO_CURSOR := V_CURSOR; 
    END GetStatesList;
    --
    --
	-- Details procedures.
	-- These get details for linecuts, jumpers and tags.
	-- Note that for opstate, we already have all the details we need from the initial GetStates query.
	--
    PROCEDURE GetLinecutDetails (
		IN_OBJID		IN VARCHAR2, 
		OUT_LINE_ID		IN OUT VARCHAR2, 
		OUT_DIST		IN OUT NUMBER
    )
    IS 
		LINE_FPOS NUMBER;
		
    BEGIN 
		-- TODO calculate OUT_DIST
		select li_key, 50 into LINE_FPOS, OUT_DIST from device where DID = IN_OBJID;
		select ID into out_line_id from line where fpos = LINE_FPOS;
    END GetLinecutDetails;
    --
    PROCEDURE GetJumperlineDetails (
		IN_OBJID		IN VARCHAR2, 
		OUT_NODE1_ID	IN OUT VARCHAR2, 
		OUT_NODE2_ID	IN OUT VARCHAR2,
		OUT_CU			IN OUT NUMBER
    )
    IS 
		NODE1_FPOS NUMBER;
		NODE2_FPOS NUMBER;
		
    BEGIN 
		-- TODO do CU and LINE_TYPE align?  If not, we need to map.
		select no_key_1, no_key_2, line_type into NODE1_FPOS, NODE2_FPOS, OUT_CU from line where ID = IN_OBJID;
		select NID into out_node1_id from node where nfpos = NODE1_FPOS;
		select NID into out_node2_id from node where nfpos = NODE2_FPOS;
    END GetJumperlineDetails;
    --
    PROCEDURE GetTagDetails (
		IN_USERTAGID	IN VARCHAR2, 
		OUT_PLACEDBY	IN OUT VARCHAR2,
		OUT_COMPANY		IN OUT VARCHAR2,
		OUT_PHONE		IN OUT VARCHAR2,
		OUT_WORK		IN OUT VARCHAR2,
		OUT_BATTERY		IN OUT NUMBER,
		OUT_INDICATOR	IN OUT NUMBER,
		OUT_REMOVEDBY	IN OUT VARCHAR2,
		OUT_REMOVEDATE	IN OUT DATE,
		OUT_REFNODE		IN OUT VARCHAR2,
		OUT_NODEDIST	IN OUT NUMBER
    )
    IS 
		OBJ_FPOS NUMBER;
		OBJ_CAT  NUMBER;
		NODE_FPOS NUMBER;
		
    BEGIN 
		select 
			PLACED_BY   , HOLDER_COMPANY, CONTACT  , WORK_LOCATION, NUM_FIELD_1, NUM_FIELD_2  , REMOVED_BY   , REM_TIMESTAMP , OBJECT_FPOS, OBJECT_CAT
		into 
			OUT_PLACEDBY, OUT_COMPANY   , OUT_PHONE, OUT_WORK     , OUT_BATTERY, OUT_INDICATOR, OUT_REMOVEDBY, OUT_REMOVEDATE, OBJ_FPOS   , OBJ_CAT
		from tag_entry 
		where USER_TAG_ID = IN_USERTAGID;

		-- TODO Precisely determine refnode and nodedist if desired
		if ( OBJ_CAT = 3 ) then
			select l.NO_KEY_1 into NODE_FPOS from line l where l.fpos = OBJ_FPOS;
			select nid, 50 into OUT_REFNODE, OUT_NODEDIST from node n where n.nfpos = NODE_FPOS;
		end if;
		
    END GetTagDetails;
    --
    --
    -- Adapter configuration procedures.
	PROCEDURE SetOMSAdapterIntegrated(
		IN_INTEGRATED		IN NUMBER
	)
    IS 
		str_mode VARCHAR2(15);
    BEGIN 
		if ( IN_INTEGRATED = 1 ) then
			str_mode := 'INTEGRATED';
		else
			str_mode := 'STANDALONE';
		end if;
		merge into OMS_ADAPTER_SETTINGS oas using dual on (SETTING_NAME='MODE')
		when not matched then insert (SETTING_NAME, VALUE) values ('MODE',str_mode)
		when matched then update set VALUE = str_mode; 
    END SetOMSAdapterIntegrated;
    --
	PROCEDURE GetOMSAdapterIntegrated(
		OUT_INTEGRATED		OUT NUMBER
	)
    IS 
		str_mode VARCHAR2(15);
    BEGIN 
		select VALUE into str_mode from OMS_ADAPTER_SETTINGS where SETTING_NAME='MODE';
		if ( str_mode = 'INTEGRATED' ) then
			OUT_INTEGRATED := 1;
		else
			OUT_INTEGRATED := 0;
		end if;
    END GetOMSAdapterIntegrated;
    --
    --
    --
END DOMSSYNC_API;
/
grant EXECUTE on DOMSSYNC_API to qscwebm;
