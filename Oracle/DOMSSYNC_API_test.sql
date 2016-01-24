-- This ensures that dbms_output.put_line() shows up in SQL*Plus.
-- And that we don't overflow the default little teeny buffer.
set serveroutput on size 90000

DECLARE

	-- GetStates -- 
	subs ASTATUS1.DOMSSYNC_API.SUBSTATION_TABLE;
	obtypes ASTATUS1.DOMSSYNC_API.OBJTYPE_TABLE;

	-- GetStatesList -- 
	oblisttypes ASTATUS1.DOMSSYNC_API.OBJTYPE_TABLE;
	oblistids ASTATUS1.DOMSSYNC_API.OBJID_TABLE;
	
	-- OMSAdapterModeIntegrated -- 
	oms_adapter_integrated NUMBER;

	-- output --
	c_out ASTATUS1.DOMSSYNC_API.STATES_RESULTS_CURSOR;
	row_out ASTATUS1.DOMSSYNC_API.STATE_RESULT_RECORD;

BEGIN

	-- GetStates -- 
	
	subs(1) := 'T4035';
	subs(2) := 'T4050';
	subs(3) := 'T4070';
	subs(4) := 'T4074';
	subs(5) := 'T4108';
	subs(6) := 'T4130';
	subs(7) := 'T4150';
	subs(8) := 'T4170';
	subs(9) := 'T4190';
	
	obtypes(1) := 4;	-- switch
	obtypes(2) := 8;	-- fuse
	obtypes(3) := 3;	-- jumpers
	obtypes(4) := 26;	-- cuts
	obtypes(5) := 50;	-- tag	TODO error!
	
	ASTATUS1.DOMSSYNC_API.GetStates( subs, obtypes, c_out );
	
	dbms_output.put_line(CHR(10));	-- LF
	dbms_output.put_line('----- Abnormal IDs ----');
	LOOP
		FETCH c_out INTO row_out;
		EXIT WHEN c_out%notfound;
		dbms_output.put_line(
			     'Sub '				||	row_out.Substation 
			|| ', Object ID '		||	row_out.ObjectType 
			|| ', Object Type '		||	row_out.ObjectType 
			|| ', Equip Type '		||	row_out.EquipType		
			|| ', Equip ID '		||	row_out.EquipID		
			|| ', Tag Holder '		||	row_out.TagHolder	
			|| ', Comments '		||	substr(row_out.Comments,1,120)	-- we were overflowing output buffer
		);
	END LOOP;
	dbms_output.put_line('-----------------------');
	dbms_output.put_line(CHR(10));	-- LF


	-- GetStatesList -- 
	
	oblisttypes(1) := 18;
	oblistids(1) := '2SY71_1163131';
	oblisttypes(2) := 26;
	oblistids(2) := 'Line Cut-166602';
	oblisttypes(3) := 26;
	oblistids(3) := 'Line Cut-172452';
	oblisttypes(4) := 26;
	oblistids(4) := 'Line Cut-176559';
	oblisttypes(5) := 26;
	oblistids(5) := 'Line Cut-182618';
	oblisttypes(6) := 26;
	oblistids(6) := 'Line Cut-205846';
	oblisttypes(7) := 50;
	oblistids(7) := '041012070123';
	oblisttypes(8) := 50;
	oblistids(8) := '041012070123A';
	-- oblisttypes() := ;
	-- oblistids() := '';

	ASTATUS1.DOMSSYNC_API.GetStatesList( oblistids, oblisttypes, c_out );
	
	dbms_output.put_line(CHR(10));	-- LF
	dbms_output.put_line('----- Abnormal IDs ----');
	LOOP
		FETCH c_out INTO row_out;
		EXIT WHEN c_out%notfound;
		dbms_output.put_line(
			     'Sub '				||	row_out.Substation 
			|| ', Object ID '		||	row_out.ObjectType 
			|| ', Object Type '		||	row_out.ObjectType 
			|| ', Equip Type '		||	row_out.EquipType		
			|| ', Equip ID '		||	row_out.EquipID		
			|| ', Tag Holder '		||	row_out.TagHolder	
			|| ', Comments '		||	row_out.Comments	
		);
	END LOOP;
	dbms_output.put_line('-----------------------');
	dbms_output.put_line(CHR(10));	-- LF

	CLOSE c_out;


	-- OMSAdapterMode -- 

	dbms_output.put_line(CHR(10));	-- LF
	dbms_output.put_line('----- Set Integrated ----');
	ASTATUS1.DOMSSYNC_API.SetOMSAdapterIntegrated(1);
	ASTATUS1.DOMSSYNC_API.GetOMSAdapterIntegrated(oms_adapter_integrated);
	dbms_output.put_line('Integrated set   '	||	oms_adapter_integrated);
	ASTATUS1.DOMSSYNC_API.SetOMSAdapterIntegrated(0);
	ASTATUS1.DOMSSYNC_API.GetOMSAdapterIntegrated(oms_adapter_integrated);
	dbms_output.put_line('Integrated unset '	||	oms_adapter_integrated);
	dbms_output.put_line('-----------------------');

END;
/
