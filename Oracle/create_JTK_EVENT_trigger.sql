CREATE OR REPLACE TRIGGER CAPTURE_DMS_OPS_FROM_JTK
   AFTER INSERT
   ON JTK_EVENT
   FOR EACH ROW
DECLARE
   v_integration_op VARCHAR2(40);
   v_integration_equip VARCHAR2(40);
BEGIN
	if (:new.OWNER != 'DMS' and :new.OWNER != 'SCADA' ) then

		-- TODO 
		-- if DEVICE.SCADA_CONTROL is true, this is a telemetered operation
		-- performed in OMS.  Do NOT send to DMS.

		-- we need to map OMS equip_type to integration device type
		if (:new.EQUIP_TYPE = 4) then
			v_integration_equip := 'switch';
		elsif (:new.EQUIP_TYPE = 8) then
			v_integration_equip := 'load';
		elsif (:new.EQUIP_TYPE = 17) then
			v_integration_equip := 'relay';
		elsif (:new.EQUIP_TYPE = 18) then
			v_integration_equip := 'fuse';
		elsif (:new.EQUIP_TYPE = 19) then
			v_integration_equip := 'recloser';
		else
			v_integration_equip := 'unknown';
			-- TODO log error and exit
		end if;
		
		-- todo
		-- Expand to include all varieties of these verbs.
		if (:new.OPERATION = 'Open') or (:new.OPERATION = 'Close') or (:new.OPERATION = 'Bypass On') or (:new.OPERATION = 'Bypass Off') or (:new.OPERATION = 'Connect') or (:new.OPERATION = 'Disconnect') then

			-- we need to map OMS operation name to integration operation name
			if (:new.OPERATION = 'Open') then
				v_integration_op := 'open';
			elsif (:new.OPERATION = 'Close') then
				v_integration_op := 'close';
			elsif (:new.OPERATION = 'Connect') then
				v_integration_op := 'close';
			elsif (:new.OPERATION = 'Disconnect') then
				v_integration_op := 'open';
			elsif (:new.OPERATION = 'Bypass On') then
				v_integration_op := 'bypass';
			elsif (:new.OPERATION = 'Bypass Off') then
				v_integration_op := 'unbypass';
			end if;

			insert into OMS_TO_DMS_OPSTATECHANGE (
				EV_TIMESTAMP,
				OWNER,
				OPERATION,
				EQUIP_TYPE,
				EQUIP_ID,
				PHASES,
				PROCESSED
			) values(
				:new.EV_TIMESTAMP,
				:new.OWNER,
				v_integration_op,
				v_integration_equip,
				:new.EQUIP_ID,
				:new.PHASES,
				0
			);

		elsif (:new.OPERATION = 'Cut') or (:new.OPERATION = 'Remove Cut') or (:new.OPERATION = 'Jumper Line') or (:new.OPERATION = 'Remove') then

			-- we need to map OMS operation name to integration operation name
			if (:new.OPERATION = 'Cut') then
				v_integration_op := 'linecut add';
			elsif (:new.OPERATION = 'Remove Cut') then
				v_integration_op := 'linecut remove';
			elsif (:new.OPERATION = 'Jumper Line') then
				v_integration_op := 'jumperline add';
			elsif (:new.OPERATION = 'Remove') then
				v_integration_op := 'jumperline remove';
			end if;

			insert into OMS_TO_DMS_TEMPOBJ (
				EV_TIMESTAMP,
				OWNER,
				OPERATION,
				EQUIP_TYPE,
				EQUIP_ID,
				PHASES,
				NODE_ID,
				NODE2_ID,
				PROCESSED
			) values(
				:new.EV_TIMESTAMP,
				:new.OWNER,
				v_integration_op,
				v_integration_equip,
				:new.EQUIP_ID,
				:new.PHASES,
				:new.FPOS_1,
				:new.FPOS_2,
				0
			);
		end if;
	end if;
exception
   when others then
			null; -- don't do anything to cause an exception
END;
/
