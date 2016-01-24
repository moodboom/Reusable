-- ---------------------------------------------------------------------------------------------------
--
-- This procedure calls other procedures in the new OPSTATE API from ABB.
--
-- The purpose is to verify both the functionality and the response times of the API procedures.
--
-- We call the API procedures with a variety of data.
-- We loop through calls at a predetermined rate and verify that the operations take place without
-- degrading the system performance.
-- ---------------------------------------------------------------------------------------------------
--
-- We need to grant access to SYS.dbms_lock.sleep
-- Only grant to the user that needs it (ie don't use PUBLIC)
-- Currently, ASTORM1 has the privilege and can install the package.
--
-- Can't do the grant here if you don't already have it, doh
-- But here's an example:
-- grant EXECUTE ON DBMS_LOCK TO ASTORM1;
--
create or replace
procedure TEST_OPSTATE_API
is
		myVar VARCHAR2(100);
begin
	FOR N in 1..10 LOOP
		ASTORM1.OUTAGE_API.OPEN_DEVICE(
			'astatus1', 
			'NRA40_2194526', 
			'AC', 
			'27-Jul-2010 09:38:17', 
			'MDM testing', 
			'DMS'				-- this prevents test from flowing to DMS
		);
		commit;
		SYS.dbms_lock.sleep(1) ;
		myVar := DBMESSAGER.SEND_DEVICE_OPEN_REQUEST(
			'astatus1', 
			'NRA40_2194526', 
			101, 
			2, 
			'07/27/2010 09:38:17', 
			'MDM testing', 
			'DMS'				-- this prevents test from flowing to DMS
		);
		commit;
		SYS.dbms_lock.sleep(1) ;
	END LOOP;
end;
/
