-------------------------------------------------------------------------------------------------------------------
--				Author	: 	Yashaswini G S(EYASHGS)
--				Date	: 	23-FEB-2019
--				Purpose	: 	Formatter for SBC-Movius MVCF INTERIM
--				Revision: 	A
----------------------------------------------------------------------------------------------------------------
-- -- Revision history:
--	Rev			Date			By					Comments
-- ----	----------		------------	-----------------------------------	------------------------
--	2020/01/22		B				oe738112	Trimming the right of dialed digit to make the length 31 bytes
--	2020/02/11		c				mj290122	Trimming the right of called_num to make the length 15 bytes
--  2021/08/17		D  				oi336594/EDATANI	Avoid processing VSA-202 for only special character and alphabetic value
----------------------------------------------------------------------------------------------------------------

IA5String numberNormalization(sCallNumPar CONST IA5String, sTypePar CONST IA5String, bIsTelPar CONST BOOLEAN);
IA5String getDateTimeFormatConversion(tmpStrPar CONST IA5String);
INTEGER checkNumeric(sValPar CONST IA5String, iLenPar CONST INTEGER);
IA5String removeNonNumberAndZero(sValPar CONST IA5String) ;
CONST INTEGER main(theCallNr CONST INTEGER, theTarget Sprint_SBC_Movius_OUTPUT.SBCMovius_MVCF_Interim, theSource CONST Sprint_SBC_Movius_INPUT.Sbc_Movius_Interim)
{
	declare oTmpOctStr OCTET STRING;
	declare tmp_mobile_role INTEGER;
	declare iTmp  INTEGER;
	declare iSize INTEGER;
	declare sInputDate IA5String;
	declare sTmpMobileNum IA5String;
	declare sVar IA5String;
	declare iIndex1 INTEGER;
	declare iIndex2 INTEGER;
	declare sEgressFinalRoutingNumber IA5String;
	declare sTmpuser_data IA5String;
	declare octTmp OCTET STRING;
	declare sTmpErrorData IA5String;
	declare dummyResult IA5String;
	declare acmeCustomVSA201 IA5String;
	declare searchStr IA5String;
	declare searchSpcharVSA-202 IA5String;
	declare searchAlphaVSA-202 IA5String;
	
	declare startPos INTEGER;
	declare sHex_str IA5String;
	declare oHex_str OCTET STRING;
	declare oHex_str1 OCTET STRING;
	declare sTmpUserData IA5String;
	declare acmeCustomVSA200 IA5String;
	declare endPos INTEGER;
	declare sHex_str1 IA5String;
	declare sToFind IA5String;
	declare sPos IA5String;
	declare iCharCount INTEGER;
	declare sFinalStr IA5String;
	declare tmpCalledNum IA5String;
	declare sTmpMobileNum2 IA5String;
	declare sTrunkGroupNumber IA5String;
	declare sTrunkgrouplong IA5String;

-- Set default values
	theTarget.overseas ::= 0;
	
	
	if(isDefined(theSource.neid) && theSource.neid !="")
	{
		oTmpOctStr ::= theSource.neid;
		theTarget.first_networkelementid ::= string2Integer(oTmpOctStr);
	} else
		theTarget.first_networkelementid ::= 0;
	
	
	clear(theTarget.last_networkelementid) ;
	
	if(isDefined(theSource.processingDate))
	{
		oTmpOctStr ::= theSource.processingDate;
		theTarget.db_host_process_datetime ::= string2Integer(oTmpOctStr);
	} else
		theTarget.db_host_process_datetime ::= 0;

	theTarget.switch_type ::= 41;
	
	theTarget.mobile_role ::= 0;
	theTarget.calling_num ::= "";
	if(isDefined(theSource.acme-P-Asserted-Id) && (theSource.acme-P-Asserted-Id) !="")
	{
		sTmpMobileNum ::= numberNormalization(theSource.acme-P-Asserted-Id,"mobile_num",FALSE);
		if (left(sTmpMobileNum,1,8) == "1")
			sTmpMobileNum ::= removeDigits(sTmpMobileNum,0,1,8);
		else if(sTmpMobileNum =="unavailable")
			sTmpMobileNum ::= "";
			
		theTarget.calling_num ::= sTmpMobileNum;
	} 
	
	theTarget.mobile_num ::= theTarget.calling_num;
	----------
	theTarget.dialed_digits ::= numberNormalization(theSource.acme-Egress-Final-Routing-Number,"mobile_num",FALSE);

	tmpCalledNum ::= "";
	tmp_mobile_role ::= 1;
	startPos ::= -1;
	endPos ::= -1;
		
	sEgressFinalRoutingNumber ::= theSource.acme-Egress-Final-Routing-Number;
	searchStr ::= "rn=";
	
--Rev D change starts	
	searchSpcharVSA-202 ::= "[0-9]";
	searchAlphaVSA-202 ::="[a-zA-Z]";
	if(isDefined(theSource.acme-Custom-VSA-202) && (theSource.acme-Custom-VSA-202) != " " &&  findIndex(theSource.acme-Custom-VSA-202,TRUE,searchSpcharVSA-202,0) > 0 &&  findIndex(theSource.acme-Custom-VSA-202,TRUE,searchAlphaVSA-202,0) < 0 ) ----US81915 : acme-Custom-VSA-201 changed to acme-Custom-VSA-202
	{
--Rev D change Ends here
		--__print("Going Into IF  BLOCK!!!");
		sVar ::= theSource.acme-Custom-VSA-202; 					----US81915 : acme-Custom-VSA-201 changed to acme-Custom-VSA-202
			for (iTmp ::=0; iTmp<sizeof(sVar); iTmp+=1)
			{
			sPos::= mid(sVar,iTmp,1,8);
				if(!(sPos IN ["0","1","2","3","4","5","6","7","8","9"]))
				{
				iCharCount += 1;
				sFinalStr ::= mid(sVar,iCharCount,sizeof(sVar),8);
				}
				else
				break;
			}
		tmpCalledNum ::= sFinalStr;
	}
	else if(isDefined(theSource.acme-Egress-Final-Routing-Number) && findIndex(sEgressFinalRoutingNumber,FALSE,searchStr,0) >= 0)
	{
			--__print("Going Into ELSE  BLOCK!!!");
			startPos ::= findIndex(sEgressFinalRoutingNumber,FALSE,searchStr,0);
			startPos += 3;
			searchStr ::= "@";
			if(findIndex(sEgressFinalRoutingNumber, FALSE, searchStr, startPos) != -1)
				iIndex1 ::= findIndex(sEgressFinalRoutingNumber, FALSE, searchStr, startPos);
			
			searchStr ::= ";";
			if(findIndex(sEgressFinalRoutingNumber, FALSE, searchStr, startPos) != -1)
				iIndex2 ::= findIndex(sEgressFinalRoutingNumber, FALSE, searchStr, startPos);
			
			if(iIndex1 != -1 && iIndex2 != -1)
			{
				if(iIndex1 > iIndex2)
					endPos ::= iIndex2;
				else
					endPos ::= iIndex1;
			}
			else if(iIndex1 == -1 && iIndex2 != -1)
				endPos ::= iIndex2;
			else if(iIndex1 != -1 && iIndex2 == -1)
					endPos ::= iIndex1;

			if(startPos != -1 && endPos != -1)
				endPos -= startPos;
			else if(startPos != -1 && endPos == -1)
				endPos ::= sizeof(sEgressFinalRoutingNumber);
			else
			{
				startPos ::= 0;
				endPos ::= 0;
			}

			tmpCalledNum ::= mid(sEgressFinalRoutingNumber, startPos, endPos, 8);
		
			tmpCalledNum ::= removeNonNumberAndZero(tmpCalledNum);

	}
	else
		tmpCalledNum ::= theTarget.dialed_digits;
	
	if((sizeof(tmpCalledNum) >= 10  && sizeof(theTarget.dialed_digits) >= 10) && (right(theTarget.dialed_digits,10,8) ==  right(tmpCalledNum,10,8)))
		tmp_mobile_role ::= 0;
	
	theTarget.mobile_role ::= tmp_mobile_role;
	
	if(left(tmpCalledNum,1,8) == "1")
			tmpCalledNum ::= removeDigits(tmpCalledNum,0,1,8);
	theTarget.called_num ::= tmpCalledNum; 
	
	-- Rev C: Code changes starts for called_num length fix 
	if (isDefined(theTarget.called_num) && theTarget.called_num != "")
	{
		if (sizeof(theTarget.called_num > 15))
		{
			theTarget.called_num ::= left(theTarget.called_num,15,8);
		}
	}
	--Rev C: Code changes ends for called_num length fix	
	
	if(theTarget.mobile_role == 1)
		theTarget.mobile_num ::= theTarget.called_num;
	
	-------------------------
	sToFind ::= "+1";
	
	if(isDefined(theSource.acme-Custom-VSA-202))
	{ 
		if(findIndex(theSource.acme-Custom-VSA-202, FALSE, sToFind,0) < 0) ----US81915 : acme-Custom-VSA-201 changed to acme-Custom-VSA-202
		{
			theTarget.overseas ::= 1;
		}
	}
	
	else if(isDefined(theSource.acme-Egress-Final-Routing-Number) && (findIndex(theSource.acme-Egress-Final-Routing-Number, FALSE, "rn=",0) > 0)) --look for rn= 
	{
		if(findIndex(theSource.acme-Egress-Final-Routing-Number, FALSE, sToFind,0) < 0)
		{
			theTarget.overseas ::= 1;
		}
	}
	else
	{
		if(findIndex(theSource.acme-Egress-Final-Routing-Number, FALSE, sToFind,0) < 0)
		{
			theTarget.overseas ::= 1;
		}
	}

	theTarget.features ::= "";
	
	if (isDefined(theSource.acct-Terminate-Cause))
	{
		oTmpOctStr ::= theSource.acct-Terminate-Cause;
		iTmp ::= string2Integer(oTmpOctStr);
		if(iTmp == 1)
		{
			theTarget.call_code ::= 1;
			theTarget.term_code ::= 0;
			theTarget.rec_format_type ::= 2;
		} else if (iTmp ==4)
		{
			theTarget.term_code ::= 2;
			theTarget.call_code ::= 0;
			theTarget.rec_format_type ::= 4;
		} else if(iTmp == 5)
		{
			theTarget.term_code ::= 3;
			theTarget.call_code ::= 0;
			theTarget.rec_format_type ::= 4;
		} else if (iTmp == 17)
		{
			theTarget.term_code ::= 5;
			theTarget.call_code ::= 0;
			theTarget.rec_format_type ::= 4;
		} else
		{
			theTarget.call_code ::= 0;
			theTarget.term_code ::= 99;
			theTarget.rec_format_type ::= 4;
		}
	}

	theTarget.anchor_sid ::= 0;
	
	if(isDefined(theSource.h323-setup-time) && (theSource.h323-setup-time !=""))
	{
		sInputDate ::= theSource.h323-setup-time;
		oTmpOctStr ::= getDateTimeFormatConversion(sInputDate);
		--theTarget.orig_start_datetime ::=  string2Integer(oTmpOctStr);
		theTarget.orig_start_datetime ::= oTmpOctStr;--addded by EDATANI for DE24499
	} else
		theTarget.orig_start_datetime ::= "";--addded by EDATANI for DE24499
	
	if(isDefined(theSource.h323-setup-time) && (theSource.h323-setup-time !=""))
	{
		sInputDate ::= theSource.h323-setup-time;
		oTmpOctStr ::= getDateTimeFormatConversion(sInputDate);
		--theTarget.term_start_datetime ::=  string2Integer(oTmpOctStr);
		theTarget.term_start_datetime ::= oTmpOctStr;--addded by EDATANI for DE24499
	} else
		theTarget.term_start_datetime ::= "";--addded by EDATANI for DE24499
	
	if(isDefined(theSource.h323-connect-time) && (theSource.h323-connect-time !=""))
	{
		sInputDate ::= theSource.h323-connect-time;
		oTmpOctStr ::= getDateTimeFormatConversion(sInputDate);
		--theTarget.answer_start_datetime ::=  string2Integer(oTmpOctStr);
		theTarget.answer_start_datetime ::= oTmpOctStr;--addded by EDATANI for DE24499
	} else
		theTarget.answer_start_datetime ::= "";

-- Fix for DE25074
	if (theTarget.answer_start_datetime == "19700101000000")
	{
		theTarget.answer_start_datetime ::= "";
	}	
-- End of fix for DE25074
	
	if(isDefined(theSource.h323-disconnect-time) && (theSource.h323-disconnect-time !=""))
	{
		sInputDate ::= theSource.h323-disconnect-time;
		oTmpOctStr ::= getDateTimeFormatConversion(sInputDate);
		--theTarget.answer_stop_datetime ::=  string2Integer(oTmpOctStr);
		theTarget.answer_stop_datetime ::= oTmpOctStr;--addded by EDATANI for DE24499
	} else
		theTarget.answer_stop_datetime ::= "";--addded by EDATANI for DE24499
	
	if(isDefined(theSource.h323-disconnect-time) && (theSource.h323-disconnect-time !=""))
	{
		sInputDate ::= theSource.h323-disconnect-time;
		oTmpOctStr ::= getDateTimeFormatConversion(sInputDate);
		--theTarget.term_stop_datetime ::=  string2Integer(oTmpOctStr);
		theTarget.term_stop_datetime ::= oTmpOctStr;--addded by EDATANI for DE24499
	} else
		theTarget.term_stop_datetime ::= "";--addded by EDATANI for DE24499
	
	if(isDefined(theSource.h323-disconnect-time) && (theSource.h323-disconnect-time !=""))
	{
		sInputDate ::= theSource.h323-disconnect-time;
		oTmpOctStr ::= getDateTimeFormatConversion(sInputDate);
		--theTarget.orig_stop_datetime ::=  string2Integer(oTmpOctStr);
		theTarget.orig_stop_datetime ::= oTmpOctStr;--addded by EDATANI for DE24499
	} else
		theTarget.orig_stop_datetime ::= "";--addded by EDATANI for DE24499
	
	theTarget.ix_carrier ::= 0;

	
	theTarget.first_cell_site ::= 0; 
	
	theTarget.first_sector  ::= 0;
	
	theTarget.first_location_id  ::= "";
	
	theTarget.last_cell_site ::= 0;
	
	theTarget.last_sector ::= 0;
	
	theTarget.last_location_id ::="";
	
	theTarget.roaming_zone_id ::="";   
	
------
	theTarget.trunk_group ::= 0;
	if(isDefined(theSource.acme-Egress-Final-Routing-Number) && (theSource.acme-Egress-Final-Routing-Number) !="")
	{
		sTrunkGroupNumber ::= theSource.acme-Egress-Final-Routing-Number;
		searchStr ::= "tgrp=";
		if(findIndex(sTrunkGroupNumber,FALSE,searchStr,0) >= 0)
		{
			startPos ::= findIndex(sTrunkGroupNumber,FALSE,searchStr,0);
			iSize ::= sizeof(sTrunkGroupNumber);
			iSize -= startPos;
			sTrunkGroupNumber ::= mid(sTrunkGroupNumber,startPos,iSize,8);
			startPos ::= 5;
			searchStr ::= ";";
			if(findIndex(sTrunkGroupNumber,FALSE,searchStr,0) >= 0)
			{
				endPos ::= findIndex(sTrunkGroupNumber,FALSE,searchStr,0);
				if(endPos > startPos)
				{
					endPos -= startPos;
					sTrunkgrouplong ::= mid(sTrunkGroupNumber,startPos,endPos,8);
				}
			}
			iTmp ::= sizeof(sTrunkgrouplong);
			if(checkNumeric(sTrunkgrouplong,iTmp) == 1)
			{
				octTmp ::= sTrunkgrouplong;
				theTarget.trunk_group ::= string2Integer(octTmp);;
			}
						
			else if(isDefined(theSource.acme-Custom-VSA-200) && (theSource.acme-Custom-VSA-200 !=""))
			{
				iTmp ::= sizeof(theSource.acme-Custom-VSA-200);
				if(checkNumeric(theSource.acme-Custom-VSA-200,iTmp) == 1)
				{
					octTmp ::= theSource.acme-Custom-VSA-200;	
					theTarget.trunk_group ::= string2Integer(octTmp);
				}
				else
					theTarget.trunk_group ::= 0;
			}
		}
	
		else if(isDefined(theSource.acme-Custom-VSA-200) && (theSource.acme-Custom-VSA-200 !=""))
		{
			iTmp ::= sizeof(theSource.acme-Custom-VSA-200);
			if(checkNumeric(theSource.acme-Custom-VSA-200,iTmp) == 1)
			{
				octTmp ::= theSource.acme-Custom-VSA-200;	
				theTarget.trunk_group ::= string2Integer(octTmp);
			}
			else
				theTarget.trunk_group ::= 0;
		}
	
	}
-----	
	sTmpErrorData ::= "";
	
	if(isDefined(theSource.acct-Terminate-Cause) && (theSource.acct-Terminate-Cause) !="")
	{
		sTmpErrorData ::= theSource.acct-Terminate-Cause;
		sTmpErrorData += ";";
		theTarget.error_data ::= sTmpErrorData;
	} 
	else
		theTarget.error_data ::= sTmpErrorData;
	
	theTarget.calling_Party_Address ::=""; 
	
	theTarget.called_Party_Address ::="";
	
	theTarget.public_user_id  ::=""; 
	
	clear(theTarget.treatment_code); 
	theTarget.treatment_code ::= 9999; --addded by EDATANI for DE24499
		
	theTarget.user_data ::= "" ;
	
	theTarget.source ::= 0; 
	theTarget.block_number ::= 0; 
	theTarget.block_index ::= 0; 
	theTarget.split_rec_id ::= 1; 
	theTarget.split_rec_count ::= 1; 
	theTarget.term_np_query_status ::= 0; 
	theTarget.term_location_routing_number ::= "0";
	theTarget.session_id ::="";
	theTarget.user_session_id ::="";
	theTarget.outgoing_session_id ::="";
	theTarget.term_lrn_source ::=0; 
	theTarget.term_jurisdiction_info_parm ::="";
	theTarget.term_called_msid ::=""; 
	theTarget.term_billing_msid ::="";
	theTarget.orig_np_query_status ::=0; 
	theTarget.orig_location_routing_number ::=  "";
	theTarget.orig_jurisdiction_info_parm ::="0";
	
	theTarget.psn_ind ::="";
	theTarget.psn ::=""; 
	theTarget.imsi ::="";
	theTarget.instance_id ::=""; 
	if(isDefined(theSource.switchName))
		theTarget.switchName ::= theSource.switchName;
	if(isDefined(theSource.processingDate))
		theTarget.processingDate ::= theSource.processingDate;
	
	-- Added by edhevij for the 4 new field addition for MVCF 
	theTarget.mobile_esn ::= "";
	theTarget.mod_num ::= "0";
	theTarget.oa_code ::= "0";
	theTarget.handoff_type ::= "0";
	-- End of Added by edhevij for the 4 new field addition for MVCF 
	
	-- Rev B: Code changes starts
	if (isDefined(theTarget.dialed_digits) && theTarget.dialed_digits != "")
	{
		if (sizeof(theTarget.dialed_digits > 31))
		{
			theTarget.dialed_digits ::= left(theTarget.dialed_digits,31,8);
		}
	}
	--Rev B: Code changes ends
	return 0;
}


IA5String numberNormalization(sCallNumPar CONST IA5String, sTypePar CONST IA5String, bIsTelPar CONST BOOLEAN)
{
	declare resultStr IA5String;
	declare tmpIA5Str IA5String;
	declare tmpIA5Str1	IA5String;
	declare startPos	INTEGER;
	declare endPos		INTEGER;
	declare lookupVal	IA5String;
	declare count		INTEGER;
	declare searchStr  IA5String;
	resultStr ::= "";
	startPos ::= 0;
	endPos ::= 0;
	endPos ::= sizeof(sCallNumPar);
	--check if number starts with sip:+
	searchStr ::= "sip:+";
	if (findIndex(sCallNumPar,FALSE,searchStr,0) >= 0)
	{
		startPos ::= findIndex(sCallNumPar,FALSE,searchStr,0);
		startPos += 5;
	} 
	searchStr ::= "tel:+";
	if (findIndex(sCallNumPar,FALSE,searchStr,0) >= 0 && startPos == 0)
	{
		startPos ::= findIndex(sCallNumPar,FALSE,searchStr,0);
		startPos += 5;
	}
	
	if (startPos == 0)
	{
		searchStr ::= "sip:";
		if (findIndex(sCallNumPar,FALSE,searchStr,0) >= 0)
		{
			startPos ::= findIndex(sCallNumPar,FALSE,searchStr,0);
			startPos += 4;
		} 
		
		searchStr ::= "tel:";
		if (findIndex(sCallNumPar,FALSE,searchStr,0) >= 0  && startPos == 0)
		{
			startPos ::= findIndex(sCallNumPar,FALSE,searchStr,0);
			startPos += 4;
		}
	}
	--search for ";" or "@" in the String
	if(startPos > 0)
	{
		searchStr ::= ";";
		if (findIndex(sCallNumPar,FALSE,searchStr,0) >= 0)
		{
			endPos ::= findIndex(sCallNumPar,FALSE,searchStr,0);
			--endPos += 1;
		}
		if (endPos < startPos)
		{
			endPos ::= sizeof(sCallNumPar);
		}
		endPos -= startPos;
		tmpIA5Str ::= mid(sCallNumPar,startPos,endPos,8);
		
		searchStr ::= "@";
		startPos ::= 0;
		if (findIndex(tmpIA5Str,FALSE,searchStr,0) >= 0) 
		{
			endPos ::= findIndex(tmpIA5Str,FALSE,searchStr,0);
			--endPos += 1;
		} 
		if (endPos < startPos)
		{
			endPos ::= sizeof(tmpIA5Str);
		}
		endPos -= startPos;
		
		tmpIA5Str ::= mid(tmpIA5Str,startPos,endPos,8);
		
		resultStr ::= tmpIA5Str;
	} else
	resultStr ::= "";
	
return resultStr;
}

IA5String getDateTimeFormatConversion(tmpStrPar CONST IA5String)
{
	declare sRetDate IA5String;
	declare sConvertedTime IA5String;
	declare sConvertedDate IA5String;
	declare sMonthStr             IA5String;
	declare sArr1IA5  SEQUENCE OF IA5String;
	declare sArray1IA5 SEQUENCE OF IA5String;
	declare sTmpTime IA5String;
	--05:23:29.471 UTC Jan 23 2019
	
	sRetDate ::="";
	
	gsplit(tmpStrPar, ".",sArr1IA5);
	sTmpTime ::= sArr1IA5[0];
	gsplit(sTmpTime, ":",sArray1IA5);
	
	sConvertedTime ::= sArray1IA5[0];
	sConvertedTime += sArray1IA5[1];
	sConvertedTime += sArray1IA5[2];

	sConvertedDate ::= right(tmpStrPar,4,8);
	sMonthStr ::= mid(tmpStrPar,17,3,8);
	switch(sMonthStr)
	{
		case "Jan":
		sConvertedDate += "01";
		break;
		
		case "Feb":
		sConvertedDate += "02";
		break;
		
		case "Mar":
		sConvertedDate += "03";
		break;
		
		case "Apr":
		sConvertedDate += "04";
		break;
		
		case "May":
		sConvertedDate += "05";
		break;
		
		case "Jun":
		sConvertedDate += "06";
		break;
		
		case "Jul":
		sConvertedDate += "07";
		break;
		
		case "Aug":
		sConvertedDate += "08";
		break;
		
		case "Sep":
		sConvertedDate += "09";
		break;
		
		case "Oct":
		sConvertedDate += "10";
		break;
		
		case "Nov":
		sConvertedDate += "11";
		break;
		
		case "Dec":
		sConvertedDate += "12";
		break;
		
		default :
		raise("Invalid Month in input");
		break;
    }
    sConvertedDate += mid(tmpStrPar,21,2,8);
    sRetDate ::= sConvertedDate;
    sRetDate += sConvertedTime;
	
return sRetDate;
}



INTEGER checkNumeric(sValPar CONST IA5String, iLenPar CONST INTEGER)
{
	declare iRetVal INTEGER;
	declare iTemp INTEGER;
	declare sTemp IA5String;
	declare iCharCnt INTEGER;
	declare sInVal IA5String;
		
	sInVal ::= mid(sValPar,0,iLenPar,8);
	
	for(iTemp ::= 0 ; iTemp < sizeof(sInVal) ; iTemp += 1)
	{
		sTemp ::= mid(sInVal,iTemp,1,8);
		if(!(sTemp IN ["0","1","2","3","4","5","6","7","8","9"]))
		{
			iRetVal ::= 0;
			break;
		}
		else
		{
			iCharCnt += 1;
			if(iCharCnt == iLenPar)
				iRetVal ::= 1;				
		}
	}
	return iRetVal;
}

IA5String removeNonNumberAndZero(sValPar CONST IA5String) 
{
	declare tmpStr1 IA5String;
	declare tmpStr2 IA5String;
	declare iCounter INTEGER;

	tmpStr2 ::= sValPar; 
	tmpStr1 ::= sValPar; 	
	for (iCounter ::= 0; iCounter < sizeof(sValPar); iCounter += 1 ) 
	{
		if(tmpStr1[iCounter] IN ["1", "2", "3", "4", "5", "6", "7", "8", "9"]) 
		{
			tmpStr2 ::= mid(sValPar, iCounter, sizeof(sValPar), 8); 
			break;
		}
	}
	if(iCounter == sizeof(sValPar)) 
		tmpStr2 ::= "0";
		
	return tmpStr2;
}

