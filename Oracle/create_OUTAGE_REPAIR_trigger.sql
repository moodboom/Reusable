CREATE OR REPLACE TRIGGER CAPTURE_DMS_CUST_I_FROM_OUTREP
   AFTER INSERT
   ON OUTAGE_REPAIR
   FOR EACH ROW
DECLARE
BEGIN
	if (:new.DEV_CAT='CUSTOMER') then

		-- todo Filter only "large" customers.

		if (:new.AFRM_DONE = 'X') then
		
			insert into ASTATUS1.OMS_TO_DMS_OPSTATECHANGE (
				EV_TIMESTAMP,
				OWNER,
				OPERATION,
				EQUIP_TYPE,
				EQUIP_ID,
				PHASES,
				PROCESSED
			) values(
				:new.TIME_STAMP,
				:new.DISPATCHER,
				'open',
				'customer',
				:new.ACCNO,
				111, -- :new.BPHASES,
				0
			);

		end if;
	end if;
exception
   when others then
			null; -- todo Error handling 
			-- don't do anything to cause an exception
END;
/
CREATE OR REPLACE TRIGGER CAPTURE_DMS_CUST_U_FROM_OUTREP
   AFTER UPDATE
   ON OUTAGE_REPAIR
   FOR EACH ROW
DECLARE
   v_integration_op VARCHAR2(40);
BEGIN
	if (:new.DEV_CAT='CUSTOMER') then

		-- todo Filter only "large" customers.

		if ( NVL(:new.AFRM_DONE, '_') = 'X'  and NVL(:old.AFRM_DONE, '_')  <> 'X' ) then
		
			insert into astatus1.OMS_TO_DMS_OPSTATECHANGE (
				EV_TIMESTAMP,
				OWNER,
				OPERATION,
				EQUIP_TYPE,
				EQUIP_ID,
				PHASES,
				PROCESSED
			) values(
				:new.TIME_STAMP,
				:new.DISPATCHER,
				'open',
				'customer',
				:new.ACCNO,
				111, -- :new.BPHASES,
				0
			);

		elsif ( NVL(:new.REST_DONE, '_') = 'X'  and NVL (:old.REST_DONE, '_')  <> 'X' ) then
		
			insert into astatus1.OMS_TO_DMS_OPSTATECHANGE (
				EV_TIMESTAMP,
				OWNER,
				OPERATION,
				EQUIP_TYPE,
				EQUIP_ID,
				PHASES,
				PROCESSED
			) values(
				:new.TIME_STAMP,
				:new.DISPATCHER,
				'close',
				'customer',
				:new.ACCNO,
				111, -- :new.BPHASES,
				0
			);

		end if;
	end if;
exception
   when others then
			null; -- todo Error handling 
			-- don't do anything to cause an exception
END;
/
