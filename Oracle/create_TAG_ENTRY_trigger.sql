CREATE OR REPLACE TRIGGER CAPTURE_DMS_ADD_FROM_TAG_ENTRY
   BEFORE INSERT
   ON TAG_ENTRY
   FOR EACH ROW
DECLARE
	v_oms_prefix varchar2(5);
	v_integration_equip VARCHAR2(40);
BEGIN

	v_oms_prefix := 'OMS-';
	
	if (:new.USER_ID != 'DMS' and :new.USER_ID != 'SCADA' ) then
	
		-- convert space to NULL to process automatic underground cable failures
		if (:new.USER_TAG_ID = ' ') then
			:new.USER_TAG_ID := NULL;
		end if;
		if (:new.USER_TAG_ID is NULL) then
			:new.USER_TAG_ID := v_oms_prefix || to_char(sysdate, 'yymmdd:HH24miss');
			
			-- TODO also add a small looping sequence suffix to account for more than one tag added at the same second
			
			-- We are not ready to send an integration add.
			-- After the insert, OMS is going to send an update to FIELD_TAG_ADD_TIMESTAMP.
			-- We have to wait for that update before sending the add.
			
		end if;
	end if;
exception
   when others then
			null; -- don't do anything to cause an exception
END;
/
CREATE OR REPLACE TRIGGER CAPTURE_DMS_UPD_FROM_TAG_ENTRY
   BEFORE UPDATE
   ON TAG_ENTRY
   FOR EACH ROW
DECLARE
	oms_prefix varchar2(5);
	v_integration_op VARCHAR2(40);
	v_integration_equip VARCHAR2(40);
	v_debug VARCHAR2(40);
BEGIN

-- debug
-- why aren't we seeing this update?
-- insert into TAG_ENTRY_COPY (OBJECT_FPOS,FIELD_TAG_ADD_TIMESTAMP,TRIGGER_ACTION) values(1,:old.FIELD_TAG_ADD_TIMESTAMP, 'update old timestamp');
-- insert into TAG_ENTRY_COPY (OBJECT_FPOS,FIELD_TAG_ADD_TIMESTAMP,TRIGGER_ACTION) values(1,:new.FIELD_TAG_ADD_TIMESTAMP, 'update new timestamp');

-- -----------------------------------------------------------------
-- WARNING: THIS TRIGGER WILL ONLY CHECK CERTAIN FIELDS for changes.
-- If the integration is expanded to use more fields, 
-- make sure they are added to this initial check.
-- -----------------------------------------------------------------
-- Silly OMS is sending multiple updates for each operation.
-- Some are bogus, in that there are no field changes.
-- We need to ignore those for the sake of the integration.
if NVL(:old.FIELD_TAG_ADD_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy')) <> NVL(:new.FIELD_TAG_ADD_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy'))
or NVL(:old.USER_TAG_ID,''	) <> NVL(:new.USER_TAG_ID,'')
or NVL(:old.STATUS,-1		) <> NVL(:new.STATUS,-1		)
or NVL(:old.OBJECT_CAT,-1	) <> NVL(:new.OBJECT_CAT,-1	)
or NVL(:old.NUM_FIELD_1,-1	) <> NVL(:new.NUM_FIELD_1,-1)
or NVL(:old.NUM_FIELD_2,-1	) <> NVL(:new.NUM_FIELD_2,-1)
or NVL(:old.TAG_LEVEL,-1	) <> NVL(:new.TAG_LEVEL,-1	)
or NVL(:old.USER_ID,''		) <> NVL(:new.USER_ID,''	)
or NVL(:old.OBJECT_ID,''	) <> NVL(:new.OBJECT_ID,''	)
or NVL(:old.COMMENTS,''		) <> NVL(:new.COMMENTS,''	) then

-- NOTE: we ignore FIELD_TAG_REM_TIMESTAMP changes as they are sent separately and 
-- screw up our NUM_FIELD_2 hack.  We'll exclusively use STATUS to check for tag removal.
-- Before or after the tag is removed, we will ignore timestamp changes.
-- or NVL(:old.FIELD_TAG_REM_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy')) <> NVL(:new.FIELD_TAG_REM_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy'))
-- -----------------------------------------------------------------

-- debug we're getting multiple updates (up to three) per API call
-- turns out that changing the timestamp along with any other field causes more than one update
-- always do timestamp update changes separately from any other field updates
-- if NVL(:old.FIELD_TAG_ADD_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy')) <> NVL(:new.FIELD_TAG_ADD_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy')) then
-- 	v_debug := 'new add time';
-- elsif NVL(:old.FIELD_TAG_REM_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy')) <> NVL(:new.FIELD_TAG_REM_TIMESTAMP,to_date('01/01/1940', 'mm/dd/yyyy')) then
-- 	v_debug := 'new rem time';
-- elsif NVL(:old.USER_TAG_ID,''	) <> NVL(:new.USER_TAG_ID,'') then
-- 	v_debug := 'new user tag';
-- elsif NVL(:old.STATUS,-1		) <> NVL(:new.STATUS,-1		) then
-- 	v_debug := 'new status';
-- elsif NVL(:old.OBJECT_CAT,-1	) <> NVL(:new.OBJECT_CAT,-1	) then
-- 	v_debug := 'new ob cat';
-- elsif NVL(:old.NUM_FIELD_1,-1	) <> NVL(:new.NUM_FIELD_1,-1) then
-- 	v_debug := 'new num 1';
-- elsif NVL(:old.NUM_FIELD_2,-1	) <> NVL(:new.NUM_FIELD_2,-1) then
-- 	v_debug := 'new num 2';
-- elsif NVL(:old.TAG_LEVEL,-1	) <> NVL(:new.TAG_LEVEL,-1	) then
-- 	v_debug := 'new tag level';
-- elsif NVL(:old.USER_ID,''		) <> NVL(:new.USER_ID,''	) then
-- 	v_debug := 'new user id';
-- elsif NVL(:old.OBJECT_ID,''	) <> NVL(:new.OBJECT_ID,''	) then
-- 	v_debug := 'new obj id';
-- elsif NVL(:old.COMMENTS,''		) <> NVL(:new.COMMENTS,''	) then
-- 	v_debug := 'new comment';
-- end if;

-- is this the update that came right after a DMS add?  if so, don't continue.
-- NOTE that we should make sure that the OMS user cannot revert FIELD_TAG_ADD_TIMESTAMP to NULL in any way.
if not (
		(:old.FIELD_TAG_ADD_TIMESTAMP is null) 
	and (:new.FIELD_TAG_ADD_TIMESTAMP is not null) 
	and (
			(:new.USER_ID = 'DMS') 
		or	(:new.USER_ID = 'SCADA')
	)
) then

	if (:new.NUM_FIELD_2 > 1) then

		-- this is an update that came from DMS
		-- fix NUM_FIELD_2 and exit so the update doesn't go round-trip
		if ( :new.NUM_FIELD_2 = 3 ) then
			:new.NUM_FIELD_2 := 1;
		else
			:new.NUM_FIELD_2 := 0;
		end if;

		-- debug
		-- insert into TAG_ENTRY_COPY (OBJECT_FPOS,NUM_FIELD_1,NUM_FIELD_2,TRIGGER_ACTION) values(1,:old.NUM_FIELD_2,:new.NUM_FIELD_2, 'caught num_field_2 hack');

	else

		-- analyze tag status 
		-- if status is 0 we don't care about this message
		if ( :new.STATUS <> 0 or :old.STATUS <> 0 ) then
		
			-- if status went from 1 to 0, this was a tag remove
			if ( :new.STATUS = 0 and :old.STATUS = 1 ) then
				v_integration_op := 'remove';
			
				-- debug
				-- insert into TAG_ENTRY_COPY (OBJECT_FPOS,STATUS,TRIGGER_ACTION) values(1,:new.STATUS, 'remove');

			-- for tag adds, OMS inserts into TAG_ENTRY, then sends a FIELD_TAG_ADD_TIMESTAMP update
			-- catch that here
			elsif :old.FIELD_TAG_ADD_TIMESTAMP is null and :new.FIELD_TAG_ADD_TIMESTAMP is not null then
				v_integration_op := 'add';

				-- TODO for line tags, gather up reference node and calc distance in percentage

				-- debug
				-- insert into TAG_ENTRY_COPY (OBJECT_FPOS,FIELD_TAG_ADD_TIMESTAMP,TRIGGER_ACTION) values(1,:new.FIELD_TAG_ADD_TIMESTAMP, 'add');

			-- else if status is 1, this is a valid update
			else
				v_integration_op := 'update';

				-- debug
				-- what is the quickest way to get a debug view of the record contents?
				-- copy to a temp table!
				-- insert into TAG_ENTRY_COPY (OBJECT_FPOS,OBJECT_ID,OBJECT_CAT,TAG_LEVEL,USER_ID,COMMENTS,USER_TAG_ID,STATUS,NUM_FIELD_1,NUM_FIELD_2,FIELD_TAG_ADD_TIMESTAMP,FIELD_TAG_REM_TIMESTAMP,TRIGGER_ACTION) 
				-- values(
				-- :old.OBJECT_FPOS,
				-- :old.OBJECT_ID,
				-- :old.OBJECT_CAT,
				-- :old.TAG_LEVEL,
				-- :old.USER_ID,
				-- :old.COMMENTS,
				-- :old.USER_TAG_ID,
				-- :old.STATUS,
				-- :old.NUM_FIELD_1,
				-- :old.NUM_FIELD_2,
				-- :old.FIELD_TAG_ADD_TIMESTAMP,
				-- :old.FIELD_TAG_REM_TIMESTAMP,
				-- 'update before'
				-- );
				-- insert into TAG_ENTRY_COPY (OBJECT_FPOS,OBJECT_ID,OBJECT_CAT,TAG_LEVEL,USER_ID,COMMENTS,USER_TAG_ID,STATUS,NUM_FIELD_1,NUM_FIELD_2,FIELD_TAG_ADD_TIMESTAMP,FIELD_TAG_REM_TIMESTAMP,TRIGGER_ACTION) 
				-- values(
				-- :new.OBJECT_FPOS,
				-- :new.OBJECT_ID,
				-- :new.OBJECT_CAT,
				-- :new.TAG_LEVEL,
				-- :new.USER_ID,
				-- :new.COMMENTS,
				-- :new.USER_TAG_ID,
				-- :new.STATUS,
				-- :new.NUM_FIELD_1,
				-- :new.NUM_FIELD_2,
				-- :new.FIELD_TAG_ADD_TIMESTAMP,
				-- :new.FIELD_TAG_REM_TIMESTAMP,
				-- 'update after'
				-- );		

			end if;
			
			-- we need to map OMS equip_type to integration device type
			if (:new.OBJECT_CAT = 2) then
				v_integration_equip := 'node';
			elsif (:new.OBJECT_CAT = 3) then
				v_integration_equip := 'line';
			elsif (:new.OBJECT_CAT = 4) then
				v_integration_equip := 'switch';
			elsif (:new.OBJECT_CAT = 8) then
				v_integration_equip := 'load';
			elsif (:new.OBJECT_CAT = 9) then
				v_integration_equip := 'source';
			elsif (:new.OBJECT_CAT = 17) then
				v_integration_equip := 'relay';
			elsif (:new.OBJECT_CAT = 18) then
				v_integration_equip := 'fuse';
			elsif (:new.OBJECT_CAT = 19) then
				v_integration_equip := 'recloser';
			elsif (:new.OBJECT_CAT = 26) then
				v_integration_equip := 'linecut';
			else
				v_integration_equip := 'unknown';
				-- TODO log error and exit
			end if;
			
			insert into OMS_TO_DMS_TAGOP (
				EV_TIMESTAMP,
				OWNER,
				OPERATION,
				TAG_ID,
				USER_TAG_ID,
				PROCESSED
			) values(
				:new.FIELD_TAG_ADD_TIMESTAMP,
				:new.USER_ID,
				
				v_integration_op,
				-- v_debug,
				
				:new.TAG_ID,			-- used to find the tag
				:new.USER_TAG_ID,
				0

				-- We can look these up later along with most other fields
				--	v_integration_equip,
				-- :new.OBJECT_ID,
			);
		
		end if;
	end if;
end if;
end if;
exception
   when others then
			null; -- todo Error handling 
			-- don't do anything to cause an exception
END;
/
