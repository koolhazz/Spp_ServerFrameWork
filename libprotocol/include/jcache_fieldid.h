#ifndef _JCACHE_FIELDID_H_
#define _JCACHE_FIELDID_H_

/******************************************************
* 这个文件定义了表的字段ID
* 不要手工修改，它由工具自动生成
******************************************************/


/**************************************************
               TblArticle
**************************************************/
const unsigned int TblArticleID = 0x0005;
const unsigned int TblArticle_UIN = 0x00050001;  //int32
const unsigned int TblArticle_ArticleID = 0x00050002;  //int32
const unsigned int TblArticle_SortId = 0x00050003;  //int32
const unsigned int TblArticle_Title = 0x00050004;  //string255
const unsigned int TblArticle_Content = 0x00050005;  //text
const unsigned int TblArticle_Flags = 0x00050006;  //int32
const unsigned int TblArticle_PostedTime = 0x00050007;  //int32
const unsigned int TblArticle_LastModTime = 0x00050008;  //int32
const unsigned int TblArticle_LastCommentTime = 0x00050009;  //int32
const unsigned int TblArticle_ArticleLabel = 0x0005000A;  //string32
const unsigned int TblArticle_ExtInfo = 0x0005000B;  //text
const unsigned int TblArticle_Summary = 0x0005000C;  //text


/**************************************************
               TblAccount
**************************************************/
const unsigned int TblAccountID = 0x0006;
const unsigned int TblAccount_Account = 0x00060001;  //string64
const unsigned int TblAccount_UIN = 0x00060002;  //int32
const unsigned int TblAccount_Password = 0x00060003;  //binary
const unsigned int TblAccount_Time = 0x00060004;  //int64
const unsigned int TblAccount_Status = 0x00060005;  //int32
const unsigned int TblAccount_ChgActPwd = 0x00060006;  //binary
const unsigned int TblAccount_chgActPwdSetTime = 0x00060007;  //int32


/**************************************************
               TblBasicInfo
**************************************************/
const unsigned int TblBasicInfoID = 0x0007;
const unsigned int TblBasicInfo_UIN = 0x00070001;  //int32
const unsigned int TblBasicInfo_Name = 0x00070002;  //string64
const unsigned int TblBasicInfo_Picture = 0x00070003;  //int32
const unsigned int TblBasicInfo_Gender = 0x00070004;  //int32
const unsigned int TblBasicInfo_Birth = 0x00070005;  //int64
const unsigned int TblBasicInfo_Blood = 0x00070006;  //int32
const unsigned int TblBasicInfo_Level = 0x00070007;  //int32
const unsigned int TblBasicInfo_Flags = 0x00070008;  //int32
const unsigned int TblBasicInfo_HomeTwon = 0x00070009;  //int32
const unsigned int TblBasicInfo_City = 0x0007000A;  //int32
const unsigned int TblBasicInfo_Version = 0x0007000B;  //int32
const unsigned int TblBasicInfo_Account = 0x0007000C;  //string64
const unsigned int TblBasicInfo_Career = 0x0007000D;  //int32


/**************************************************
               TblEmailInfo
**************************************************/
const unsigned int TblEmailInfoID = 0x0008;
const unsigned int TblEmailInfo_Email = 0x00080001;  //string64
const unsigned int TblEmailInfo_WebLink = 0x00080002;  //string255
const unsigned int TblEmailInfo_Smtp = 0x00080003;  //string255
const unsigned int TblEmailInfo_Pop = 0x00080004;  //string255


/**************************************************
               TblFreeUIN
**************************************************/
const unsigned int TblFreeUINID = 0x0009;
const unsigned int TblFreeUIN_UIN = 0x00090001;  //int32


/**************************************************
               TblArtComment
**************************************************/
const unsigned int TblArtCommentID = 0x000A;
const unsigned int TblArtComment_UIN = 0x000A0001;  //int32
const unsigned int TblArtComment_ArticleID = 0x000A0002;  //int32
const unsigned int TblArtComment_CommentID = 0x000A0003;  //int32
const unsigned int TblArtComment_PostBy = 0x000A0004;  //int32
const unsigned int TblArtComment_CommentContent = 0x000A0005;  //text
const unsigned int TblArtComment_CommentDate = 0x000A0006;  //int32
const unsigned int TblArtComment_Reply = 0x000A0007;  //int32
const unsigned int TblArtComment_Support = 0x000A0008;  //int32
const unsigned int TblArtComment_Opposed = 0x000A0009;  //int32


/**************************************************
               TblArtStat
**************************************************/
const unsigned int TblArtStatID = 0x000B;
const unsigned int TblArtStat_UIN = 0x000B0001;  //int32
const unsigned int TblArtStat_ArticleID = 0x000B0002;  //int32
const unsigned int TblArtStat_Vcnt = 0x000B0003;  //int32
const unsigned int TblArtStat_Ccnt = 0x000B0004;  //int32
const unsigned int TblArtStat_Vistor = 0x000B0005;  //string128


/**************************************************
               TblArticleSort
**************************************************/
const unsigned int TblArticleSortID = 0x000C;
const unsigned int TblArticleSort_UIN = 0x000C0001;  //int32
const unsigned int TblArticleSort_SortId = 0x000C0002;  //int32
const unsigned int TblArticleSort_SortName = 0x000C0003;  //string32
const unsigned int TblArticleSort_ArticleNum = 0x000C0004;  //int32
const unsigned int TblArticleSort_Flags = 0x000C0005;  //int32


/**************************************************
               TblDeepInfo
**************************************************/
const unsigned int TblDeepInfoID = 0x000D;
const unsigned int TblDeepInfo_UIN = 0x000D0001;  //int32
const unsigned int TblDeepInfo_InvestLevel = 0x000D0002;  //int32
const unsigned int TblDeepInfo_Attention = 0x000D0003;  //int32
const unsigned int TblDeepInfo_WorkStatus = 0x000D0004;  //int32
const unsigned int TblDeepInfo_WorkImpression = 0x000D0005;  //int32
const unsigned int TblDeepInfo_StockYears = 0x000D0006;  //int32
const unsigned int TblDeepInfo_StockOrg = 0x000D0007;  //int32
const unsigned int TblDeepInfo_InvestExperience = 0x000D0008;  //int32
const unsigned int TblDeepInfo_TotalInvestment = 0x000D0009;  //int32
const unsigned int TblDeepInfo_InvestDesire = 0x000D000A;  //int32
const unsigned int TblDeepInfo_TrustMode = 0x000D000B;  //int32
const unsigned int TblDeepInfo_SelStkMode = 0x000D000C;  //int32
const unsigned int TblDeepInfo_OperateStyle = 0x000D000D;  //int32
const unsigned int TblDeepInfo_Telephone = 0x000D000e;  //string32


/**************************************************
               TblAccountVerify
**************************************************/
const unsigned int TblAccountVerifyID = 0x000E;
const unsigned int TblAccountVerify_Account = 0x000E0001;  //string64
const unsigned int TblAccountVerify_VerifyCode = 0x000E0002;  //string64
const unsigned int TblAccountVerify_RegTime = 0x000E0003;  //int64
const unsigned int TblAccountVerify_Password = 0x000E0004;  //binary
const unsigned int TblAccountVerify_Name = 0x000E0005;  //string64
const unsigned int TblAccountVerify_Gender = 0x000E0006;  //int32
const unsigned int TblAccountVerify_Birth = 0x000E0007;  //int64
const unsigned int TblAccountVerify_InvestLevel = 0x000E0008;  //int32
const unsigned int TblAccountVerify_Attention = 0x000E0009;  //int32
const unsigned int TblAccountVerify_city = 0x000E000A;  //int32
const unsigned int TblAccountVerify_ip = 0x000E000B;  //int32
const unsigned int TblAccountVerify_old_account = 0x000E000C;  //string64
const unsigned int TblAccountVerify_type = 0x000E000D;  //int8


/**************************************************
               TblAlbum
**************************************************/
const unsigned int TblAlbumID = 0x000F;
const unsigned int TblAlbum_UIN = 0x000F0001;  //int32
const unsigned int TblAlbum_AlbumID = 0x000F0002;  //int32
const unsigned int TblAlbum_AlbumName = 0x000F0003;  //string255
const unsigned int TblAlbum_converPicPath = 0x000F0004;  //string255
const unsigned int TblAlbum_Flags = 0x000F0005;  //int32
const unsigned int TblAlbum_PicsNum = 0x000F0006;  //int32
const unsigned int TblAlbum_SpaceSize = 0x000F0007;  //int32
const unsigned int TblAlbum_Privilege = 0x000F0008;  //int32
const unsigned int TblAlbum_PostedTime = 0x000F0009;  //int32
const unsigned int TblAlbum_LastModTime = 0x000F000A;  //int32
const unsigned int TblAlbum_LastCommentTime = 0x000F000B;  //int32
const unsigned int TblAlbum_AlbumTag = 0x000F000C;  //string32
const unsigned int TblAlbum_ExtInfo = 0x000F000D;  //text


/**************************************************
               TblPicture
**************************************************/
const unsigned int TblPictureID = 0x0010;
const unsigned int TblPicture_UIN = 0x00100001;  //int32
const unsigned int TblPicture_PicID = 0x00100002;  //int32
const unsigned int TblPicture_AlbumID = 0x00100003;  //int32
const unsigned int TblPicture_uploadTime = 0x00100004;  //int32
const unsigned int TblPicture_PicPath = 0x00100005;  //string255
const unsigned int TblPicture_ThumbPath = 0x00100006;  //string255
const unsigned int TblPicture_PicComment = 0x00100007;  //string255
const unsigned int TblPicture_PicTag = 0x00100008;  //string64
const unsigned int TblPicture_PicSize = 0x00100009;  //int32
const unsigned int TblPicture_Flags = 0x0010000A;  //int32
const unsigned int TblPicture_PicWidth = 0x0010000B;  //int32
const unsigned int TblPicture_PicHeight = 0x0010000C;  //int32


/**************************************************
               TblAlbumComment
**************************************************/
const unsigned int TblAlbumCommentID = 0x0011;
const unsigned int TblAlbumComment_UIN = 0x00110001;  //int32
const unsigned int TblAlbumComment_PicID = 0x00110002;  //int32
const unsigned int TblAlbumComment_CommentID = 0x00110003;  //int32
const unsigned int TblAlbumComment_PostByUIN = 0x00110004;  //int32
const unsigned int TblAlbumComment_ReplyID = 0x00110005;  //int32
const unsigned int TblAlbumComment_Support = 0x00110006;  //int32
const unsigned int TblAlbumComment_Opposed = 0x00110007;  //int32
const unsigned int TblAlbumComment_CommentContent = 0x00110008;  //string255
const unsigned int TblAlbumComment_CommentDate = 0x00110009;  //int32


/**************************************************
               TblAlbumStat
**************************************************/
const unsigned int TblAlbumStatID = 0x0012;
const unsigned int TblAlbumStat_UIN = 0x00120001;  //int32
const unsigned int TblAlbumStat_PicID = 0x00120002;  //int32
const unsigned int TblAlbumStat_Vcnt = 0x00120003;  //int32
const unsigned int TblAlbumStat_Ccnt = 0x00120004;  //int32
const unsigned int TblAlbumStat_Vistor = 0x00120005;  //string128
const unsigned int TblAlbumStat_ExtInfo = 0x00120006;  //text


/**************************************************
               TblFriend
**************************************************/
const unsigned int TblFriendID = 0x0013;
const unsigned int TblFriend_UIN = 0x00130001;  //int32
const unsigned int TblFriend_FriendUIN = 0x00130002;  //int32
const unsigned int TblFriend_FriendFlag = 0x00130003;  //int32
const unsigned int TblFriend_GroupID = 0x00130004;  //int32
const unsigned int TblFriend_FriendMemo = 0x00130005;  //string64


/**************************************************
               TblGroup
**************************************************/
const unsigned int TblGroupID = 0x0014;
const unsigned int TblGroup_UIN = 0x00140001;  //int32
const unsigned int TblGroup_GroupID = 0x00140002;  //int32
const unsigned int TblGroup_GroupName = 0x00140003;  //string64


/**************************************************
               TblUserInfoVer
**************************************************/
const unsigned int TblUserInfoVerID = 0x0015;
const unsigned int TblUserInfoVer_UIN = 0x00150001;  //int32
const unsigned int TblUserInfoVer_BuddyList = 0x00150002;  //int32
const unsigned int TblUserInfoVer_BuddyGroup = 0x00150003;  //int32
const unsigned int TblUserInfoVer_StkList = 0x00150004;  //int32
const unsigned int TblUserInfoVer_BuddymemoList = 0x00150005;  //int32
const unsigned int TblUserInfoVer_BlackList = 0x00150006;  //int32


/**************************************************
               TblUserRights
**************************************************/
const unsigned int TblUserRightsID = 0x0016;
const unsigned int TblUserRights_UIN = 0x00160001;  //int32
const unsigned int TblUserRights_AddFriend = 0x00160002;  //int32
const unsigned int TblUserRights_ContactInfo = 0x00160003;  //int32


/**************************************************
               TblAddFriendVerify
**************************************************/
const unsigned int TblAddFriendVerifyID = 0x0017;
const unsigned int TblAddFriendVerify_UIN = 0x00170001;  //int32
const unsigned int TblAddFriendVerify_FriendUIN = 0x00170002;  //int32
const unsigned int TblAddFriendVerify_GroupID = 0x00170003;  //int32
const unsigned int TblAddFriendVerify_TimeStamp = 0x00170004;  //int32


/**************************************************
               TblSignInfo
**************************************************/
const unsigned int TblSignInfoID = 0x0018;
const unsigned int TblSignInfo_UIN = 0x00180001;  //int32
const unsigned int TblSignInfo_Sign = 0x00180002;  //string255
const unsigned int TblSignInfo_Version = 0x00180003;  //int32


/**************************************************
               TblContacts
**************************************************/
const unsigned int TblContactsID = 0x0019;
const unsigned int TblContacts_UIN = 0x00190001;  //int32
const unsigned int TblContacts_Email = 0x00190002;  //string255
const unsigned int TblContacts_Mobile = 0x00190003;  //string20
const unsigned int TblContacts_Phone = 0x00190004;  //string20
const unsigned int TblContacts_Fax = 0x00190005;  //string20
const unsigned int TblContacts_Address = 0x00190006;  //string255
const unsigned int TblContacts_QQ = 0x00190007;  //string20
const unsigned int TblContacts_MSN = 0x00190008;  //string255
const unsigned int TblContacts_Skype = 0x00190009;  //string255


/**************************************************
               TblWorks
**************************************************/
const unsigned int TblWorksID = 0x001A;
const unsigned int TblWorks_UIN = 0x001A0001;  //int32
const unsigned int TblWorks_ID = 0x001A0002;  //int32
const unsigned int TblWorks_Company = 0x001A0003;  //string255
const unsigned int TblWorks_Job = 0x001A0004;  //string255
const unsigned int TblWorks_Trade = 0x001A0005;  //int32


/**************************************************
               TblEducations
**************************************************/
const unsigned int TblEducationsID = 0x001B;
const unsigned int TblEducations_UIN = 0x001B0001;  //int32
const unsigned int TblEducations_ID = 0x001B0002;  //int32
const unsigned int TblEducations_School = 0x001B0003;  //string255
const unsigned int TblEducations_Class = 0x001B0004;  //string255
const unsigned int TblEducations_EnterTime = 0x001B0005;  //int32


/**************************************************
               TblFace
**************************************************/
const unsigned int TblFaceID = 0x001C;
const unsigned int TblFace_UIN = 0x001C0001;  //int32
const unsigned int TblFace_Face = 0x001C0002;  //binary
const unsigned int TblFace_FaceThumb = 0x001C0003;  //binary
const unsigned int TblFace_FaceHash = 0x001C0004;  //string64
const unsigned int TblFace_mtime = 0x001C0005;  //int32
const unsigned int TblFace_Flag = 0x001C0006;  //int32


/**************************************************
               TblPwdProtect
**************************************************/
const unsigned int TblPwdProtectID = 0x001D;
const unsigned int TblPwdProtect_UIN = 0x001D0001;  //int32
const unsigned int TblPwdProtect_Mobile = 0x001D0002;  //string20
const unsigned int TblPwdProtect_Email = 0x001D0003;  //string255
const unsigned int TblPwdProtect_Question1 = 0x001D0004;  //int32
const unsigned int TblPwdProtect_Answer1 = 0x001D0005;  //string255
const unsigned int TblPwdProtect_Question2 = 0x001D0006;  //int32
const unsigned int TblPwdProtect_Answer2 = 0x001D0007;  //string255
const unsigned int TblPwdProtect_Question3 = 0x001D0008;  //int32
const unsigned int TblPwdProtect_Answer3 = 0x001D0009;  //string255


/**************************************************
               TblStkGroup
**************************************************/
const unsigned int TblStkGroupID = 0x001E;
const unsigned int TblStkGroup_UIN = 0x001E0001;  //int32
const unsigned int TblStkGroup_StkGrpID = 0x001E0002;  //int32
const unsigned int TblStkGroup_StkGrpName = 0x001E0003;  //string64


/**************************************************
               TblStkInfo
**************************************************/
const unsigned int TblStkInfoID = 0x001F;
const unsigned int TblStkInfo_StkType = 0x001F0001;  //int32
const unsigned int TblStkInfo_StkCode = 0x001F0002;  //string6
const unsigned int TblStkInfo_StkName = 0x001F0003;  //string48
const unsigned int TblStkInfo_StkIndex = 0x001F0004;  //int32
const unsigned int TblStkInfo_StkShortName = 0x001F0005;  //string48
const unsigned int TblStkInfo_RefCount = 0x001F0006;  //int32


/**************************************************
               TblStkUser
**************************************************/
const unsigned int TblStkUserID = 0x0020;
const unsigned int TblStkUser_UIN = 0x00200001;  //int32
const unsigned int TblStkUser_StkGrpID = 0x00200002;  //int32
const unsigned int TblStkUser_StkType = 0x00200003;  //int32
const unsigned int TblStkUser_StkCode = 0x00200004;  //string6
const unsigned int TblStkUser_StkIdx  = 0x00200005;  //int32


/**************************************************
               TblBlacklist
**************************************************/
const unsigned int TblBlacklistID = 0x0021;
const unsigned int TblBlacklist_UIN = 0x00210001;  //int32
const unsigned int TblBlacklist_BlockUIN = 0x00210002;  //int32


/**************************************************
               TblVisitor
**************************************************/
const unsigned int TblVisitorID = 0x0022;
const unsigned int TblVisitor_UIN = 0x00220001;  //int32
const unsigned int TblVisitor_VisitorUIN = 0x00220002;  //int32
const unsigned int TblVisitor_VisitTime = 0x00220003;  //int32


/**************************************************
               TblOfflineMsg
**************************************************/
const unsigned int TblOfflineMsgID = 0x0023;
const unsigned int TblOfflineMsg_TO_UIN = 0x00230001;  //int32
const unsigned int TblOfflineMsg_MsgTime_Micro = 0x00230002;  //int64
const unsigned int TblOfflineMsg_MsgTime = 0x00230003;  //int32
const unsigned int TblOfflineMsg_Msg = 0x00230004;  //binary


/**************************************************
               TblGrpOfflineMsg
**************************************************/
const unsigned int TblGrpOfflineMsgID = 0x0024;
const unsigned int TblGrpOfflineMsg_TO_UIN = 0x00240001;  //int32
const unsigned int TblGrpOfflineMsg_MsgTime_Micro = 0x00240002;  //int64
const unsigned int TblGrpOfflineMsg_MsgTime = 0x00240003;  //int32
const unsigned int TblGrpOfflineMsg_GroupID = 0x00240004;  //int32
const unsigned int TblGrpOfflineMsg_Msg = 0x00240005;  //binary


/**************************************************
               TblSysMsg
**************************************************/
const unsigned int TblSysMsgID = 0x0025;
const unsigned int TblSysMsg_MsgID = 0x00250001;  //int32
const unsigned int TblSysMsg_IsValid = 0x00250002;  //int8
const unsigned int TblSysMsg_OptType = 0x00250003;  //int8
const unsigned int TblSysMsg_SendType = 0x00250004;  //int8
const unsigned int TblSysMsg_ReSendFlag = 0x00250005;  //int8
const unsigned int TblSysMsg_SendTime = 0x00250006;  //int32
const unsigned int TblSysMsg_StartTime = 0x00250007;  //int32
const unsigned int TblSysMsg_EndTime = 0x00250008;  //int32
const unsigned int TblSysMsg_SendDestType = 0x00250009;  //int8
const unsigned int TblSysMsg_DestUser = 0x0025000A;  //int8
const unsigned int TblSysMsg_MsgType = 0x0025000B;  //int8
const unsigned int TblSysMsg_Msg = 0x0025000C;  //binary
const unsigned int TblSysMsg_InsertTime = 0x0025000D;  //int32
const unsigned int TblSysMsg_UserName = 0x0025000E;  //string64
const unsigned int TblSysMsg_Remark = 0x0025000F;  //string256


/**************************************************
               TblDiscussInfo
**************************************************/
const unsigned int TblDiscussInfoID = 0x0026;
const unsigned int TblDiscussInfo_GroupUIN = 0x00260001;  //int32
const unsigned int TblDiscussInfo_Name = 0x00260002;  //string64
const unsigned int TblDiscussInfo_Flags = 0x00260003;  //int32
const unsigned int TblDiscussInfo_CreateTime = 0x00260004;  //int32
const unsigned int TblDiscussInfo_Owner = 0x00260005;  //int32
const unsigned int TblDiscussInfo_Status = 0x00260006;  //int32
const unsigned int TblDiscussInfo_InfoVer = 0x00260007;  //int32
const unsigned int TblDiscussInfo_LastActiveTime = 0x00260008;  //int32


/**************************************************
               TblGrpBasicInfo
**************************************************/
const unsigned int TblGrpBasicInfoID = 0x0027;
const unsigned int TblGrpBasicInfo_GroupUIN = 0x00270001;  //int32
const unsigned int TblGrpBasicInfo_Name = 0x00270002;  //string128
const unsigned int TblGrpBasicInfo_Level = 0x00270003;  //int32
const unsigned int TblGrpBasicInfo_ParentClass = 0x00270004;  //int32
const unsigned int TblGrpBasicInfo_SubClass = 0x00270005;  //int32
const unsigned int TblGrpBasicInfo_Flags = 0x00270006;  //int32
const unsigned int TblGrpBasicInfo_Setting = 0x00270007;  //int32
const unsigned int TblGrpBasicInfo_Owner = 0x00270008;  //int32
const unsigned int TblGrpBasicInfo_Face = 0x00270009;  //int32
const unsigned int TblGrpBasicInfo_Intro = 0x0027000a;  //text
const unsigned int TblGrpBasicInfo_Board = 0x0027000b;  //text
const unsigned int TblGrpBasicInfo_CreateTime = 0x0027000c;  //int32
const unsigned int TblGrpBasicInfo_Status = 0x0027000d;  //int32
const unsigned int TblGrpBasicInfo_InfoVer = 0x0027000e;  //int32
const unsigned int TblGrpBasicInfo_FaceVer = 0x0027000f;  //int32
const unsigned int TblGrpBasicInfo_MemberCount = 0x00270010;  //int32
const unsigned int TblGrpBasicInfo_Creator = 0x00270011;  //int32


/**************************************************
               TblGrpMember
**************************************************/
const unsigned int TblGrpMemberID = 0x0028;
const unsigned int TblGrpMember_GroupUIN = 0x00280001;  //int32
const unsigned int TblGrpMember_UIN = 0x00280002;  //int32
const unsigned int TblGrpMember_Flags = 0x00280003;  //int32
const unsigned int TblGrpMember_JoinTime = 0x00280004;  //int32


/**************************************************
               TblGrpFreeUIN
**************************************************/
const unsigned int TblGrpFreeUINID = 0x0029;
const unsigned int TblGrpFreeUIN_GroupUIN = 0x00290001;  //int32


/**************************************************
               TblGrpFace
**************************************************/
const unsigned int TblGrpFaceID = 0x002a;
const unsigned int TblGrpFace_GroupUIN = 0x002a0001;  //int32
const unsigned int TblGrpFace_Face = 0x002a0002;  //binary
const unsigned int TblGrpFace_FaceThumb = 0x002a0003;  //binary
const unsigned int TblGrpFace_FaceHash = 0x002a0004;  //string64


/**************************************************
               TblGrpInvite
**************************************************/
const unsigned int TblGrpInviteID = 0x002b;
const unsigned int TblGrpInvite_FromUIN = 0x002b0001;  //int32
const unsigned int TblGrpInvite_ToUIN = 0x002b0002;  //int32
const unsigned int TblGrpInvite_GroupUIN = 0x002b0003;  //int32
const unsigned int TblGrpInvite_InviteTime = 0x002b0004;  //int32


/**************************************************
               TblGrpRequest
**************************************************/
const unsigned int TblGrpRequestID = 0x002c;
const unsigned int TblGrpRequest_FromUIN = 0x002C0001;  //int32
const unsigned int TblGrpRequest_GroupUIN = 0x002C0002;  //int32
const unsigned int TblGrpRequest_RequestTime = 0x002C0003;  //int32


/**************************************************
               TblFamiliar
**************************************************/
const unsigned int TblFamiliarID = 0x002d;
const unsigned int TblFamiliar_UIN = 0x002D0001;  //int32
const unsigned int TblFamiliar_FamiliarUIN = 0x002D0002;  //int32
const unsigned int TblFamiliar_Type = 0x002D0003;  //int32
const unsigned int TblFamiliar_Time = 0x002D0004;  //int32


/**************************************************
               TblMyVisited
**************************************************/
const unsigned int TblMyVisitedID = 0x002e;
const unsigned int TblMyVisited_UIN = 0x002e0001;  //int32
const unsigned int TblMyVisited_SpaceUIN = 0x002e0002;  //int32
const unsigned int TblMyVisited_VisitTime = 0x002e0003;  //int32


/**************************************************
               TblNoteInBox
**************************************************/
const unsigned int TblNoteInBoxID = 0x002f;
const unsigned int TblNoteInBox_NoteID = 0x002f0001;  //int32
const unsigned int TblNoteInBox_UIN = 0x002f0002;  //int32
const unsigned int TblNoteInBox_FromUIN = 0x002f0003;  //int32
const unsigned int TblNoteInBox_Content = 0x002f0004;  //string600
const unsigned int TblNoteInBox_PostTime = 0x002f0005;  //int32
const unsigned int TblNoteInBox_IsPrivate = 0x002f0006;  //int8
const unsigned int TblNoteInBox_IsRead = 0x002f0007;  //int8
const unsigned int TblNoteInBox_ParentNote = 0x002f0008;  //int32


/**************************************************
               TblNoteOutBox
**************************************************/
const unsigned int TblNoteOutBoxID = 0x0030;
const unsigned int TblNoteOutBox_NoteID = 0x00300001;  //int32
const unsigned int TblNoteOutBox_UIN = 0x00300002;  //int32
const unsigned int TblNoteOutBox_ToUIN = 0x00300003;  //int32
const unsigned int TblNoteOutBox_Content = 0x00300004;  //string600
const unsigned int TblNoteOutBox_PostTime = 0x00300005;  //int32
const unsigned int TblNoteOutBox_IsPrivate = 0x00300006;  //int8


/**************************************************
               TblWebSysMsg
**************************************************/
const unsigned int TblWebSysMsgID = 0x0031;
const unsigned int TblWebSysMsg_MsgID = 0x00310001;  //int32
const unsigned int TblWebSysMsg_UIN = 0x00310002;  //int32
const unsigned int TblWebSysMsg_FromUIN = 0x00310003;  //int32
const unsigned int TblWebSysMsg_Content = 0x00310004;  //string512
const unsigned int TblWebSysMsg_sendTime = 0x00310005;  //int32
const unsigned int TblWebSysMsg_IsRead = 0x00310006;  //int8
const unsigned int TblWebSysMsg_Type = 0x00310007;  //int8


/**************************************************
               TblFriendReqMsg
**************************************************/
const unsigned int TblFriendReqMsgID = 0x0032;
const unsigned int TblFriendReqMsg_MsgID = 0x00320001;  //int32
const unsigned int TblFriendReqMsg_UIN = 0x00320002;  //int32
const unsigned int TblFriendReqMsg_RequestUIN = 0x00320003;  //int32
const unsigned int TblFriendReqMsg_Content = 0x00320004;  //string255
const unsigned int TblFriendReqMsg_sendTime = 0x00320005;  //int32
const unsigned int TblFriendReqMsg_IsRead = 0x00320006;  //int8
const unsigned int TblFriendReqMsg_IsDone = 0x00320007;  //int8
const unsigned int TblFriendReqMsg_Type = 0x00320008;  //int8


/**************************************************
               TblFriendNews
**************************************************/
const unsigned int TblFriendNewsID = 0x0033;
const unsigned int TblFriendNews_ID = 0x00330001;  //int32
const unsigned int TblFriendNews_UIN = 0x00330002;  //int32
const unsigned int TblFriendNews_RelatedUIN = 0x00330003;  //int32
const unsigned int TblFriendNews_RelatedID = 0x00330004;  //int32
const unsigned int TblFriendNews_Content = 0x00330005;  //text
const unsigned int TblFriendNews_SubmitTime = 0x00330006;  //int32
const unsigned int TblFriendNews_Type = 0x00330007;  //int32
const unsigned int TblFriendNews_reserved_1 = 0x00330008;  //string255
const unsigned int TblFriendNews_reserved_2 = 0x00330009;  //string255


/**************************************************
               TblDiscussMember
**************************************************/
const unsigned int TblDiscussMemberID = 0x0034;
const unsigned int TblDiscussMember_GroupUIN = 0x00340001;  //int32
const unsigned int TblDiscussMember_UIN = 0x00340002;  //int32
const unsigned int TblDiscussMember_Flags = 0x00340003;  //int32
const unsigned int TblDiscussMember_JoinTime = 0x00340004;  //int32
const unsigned int TblDiscussMember_Type = 0x00340005;  //int32


/**************************************************
               TblDiscussFreeUIN
**************************************************/
const unsigned int TblDiscussFreeUINID = 0x0035;
const unsigned int TblDiscussFreeUIN_GroupUIN = 0x00350001;  //int32


/**************************************************
               TblGrpList
**************************************************/
const unsigned int TblGrpListID = 0x0036;
const unsigned int TblGrpList_UIN = 0x00360001;  //int32
const unsigned int TblGrpList_GroupUIN = 0x00360002;  //int32


/**************************************************
               TblDiscussList
**************************************************/
const unsigned int TblDiscussListID = 0x0037;
const unsigned int TblDiscussList_UIN = 0x00370001;  //int32
const unsigned int TblDiscussList_GroupUIN = 0x00370002;  //int32


/**************************************************
               TblDiscOfflineMsg
**************************************************/
const unsigned int TblDiscOfflineMsgID = 0x0038;
const unsigned int TblDiscOfflineMsg_TO_UIN = 0x00380001;  //int32
const unsigned int TblDiscOfflineMsg_MsgTime_Micro = 0x00380002;  //int64
const unsigned int TblDiscOfflineMsg_MsgTime = 0x00380003;  //int32
const unsigned int TblDiscOfflineMsg_DiscussID = 0x00380004;  //int32
const unsigned int TblDiscOfflineMsg_Msg = 0x00380005;  //binary


/**************************************************
               TblFriendInvite
**************************************************/
const unsigned int TblFriendInviteID = 0x0039;
const unsigned int TblFriendInvite_email_addr = 0x00390001;  //string128
const unsigned int TblFriendInvite_Inviter_uin = 0x00390002;  //int32
const unsigned int TblFriendInvite_Email_type = 0x00390003;  //int32
const unsigned int TblFriendInvite_Invite_time = 0x00390004;  //int32
const unsigned int TblFriendInvite_Status = 0x00390005;  //int32


/**************************************************
               TblAppInfo
**************************************************/
const unsigned int TblAppInfoID = 0x0040;
const unsigned int TblAppInfo_AppId = 0x00400001;  //int32
const unsigned int TblAppInfo_AppName = 0x00400002;  //string255
const unsigned int TblAppInfo_AppDesc = 0x00400003;  //text
const unsigned int TblAppInfo_AppIconIndex = 0x00400004;  //int8
const unsigned int TblAppInfo_AppLevel = 0x00400005;  //int8
const unsigned int TblAppInfo_Status = 0x00400006;  //int8
const unsigned int TblAppInfo_CreateUin = 0x00400007;  //int32
const unsigned int TblAppInfo_CreateTime = 0x00400008;  //int32
const unsigned int TblAppInfo_AppExType = 0x00400009;  //int32
const unsigned int TblAppInfo_AppExInfo = 0x0040000a;  //text


/**************************************************
               TblAppUser
**************************************************/
const unsigned int TblAppUserID = 0x0041;
const unsigned int TblAppUser_UIN = 0x00410001;  //int32
const unsigned int TblAppUser_AppId = 0x00410002;  //int32
const unsigned int TblAppUser_AddTime = 0x00410003;  //int32
const unsigned int TblAppUser_InviteUin = 0x00410004;  //int32


/**************************************************
               TblFeedsSetting
**************************************************/
const unsigned int TblFeedsSettingID = 0x0042;
const unsigned int TblFeedsSetting_UIN = 0x00420001;  //int32
const unsigned int TblFeedsSetting_AppId = 0x00420002;  //int32
const unsigned int TblFeedsSetting_SendFeeds = 0x00420003;  //int8
const unsigned int TblFeedsSetting_RecvFeeds = 0x00420004;  //int8


/**************************************************
               TblRightsSetting
**************************************************/
const unsigned int TblRightsSettingID = 0x0043;
const unsigned int TblRightsSetting_UIN = 0x00430001;  //int32
const unsigned int TblRightsSetting_RightsId = 0x00430002;  //int32
const unsigned int TblRightsSetting_value = 0x00430003;  //int8


/**************************************************
               TblDiscussRecycleUIN
**************************************************/
const unsigned int TblDiscussRecycleUINID = 0x0044;
const unsigned int TblDiscussRecycleUIN_GroupUIN = 0x00440001;  //int32


/**************************************************
               TblGrpRecycleUIN
**************************************************/
const unsigned int TblGrpRecycleUINID = 0x0045;
const unsigned int TblGrpRecycleUIN_GroupUIN = 0x00450001;  //int32


/**************************************************
               TblMyMood
**************************************************/
const unsigned int TblMyMoodID = 0x0046;
const unsigned int TblMyMood_uin = 0x00460001;  //int32
const unsigned int TblMyMood_MoodId = 0x00460002;  //int32
const unsigned int TblMyMood_FromUIN = 0x00460003;  //int32
const unsigned int TblMyMood_Content = 0x00460004;  //string800
const unsigned int TblMyMood_PostTime = 0x00460005;  //int32
const unsigned int TblMyMood_IsRead = 0x00460006;  //int32
const unsigned int TblMyMood_ParentId = 0x00460007;  //int32
const unsigned int TblMyMood_expressionId = 0x00460008;  //int32


/**************************************************
               TblFriendMood
**************************************************/
const unsigned int TblFriendMoodID = 0x0047;
const unsigned int TblFriendMood_uin = 0x00470001;  //int32
const unsigned int TblFriendMood_MoodId = 0x00470002;  //int32
const unsigned int TblFriendMood_FriendUIN = 0x00470003;  //int32
const unsigned int TblFriendMood_PostTime = 0x00470004;  //int32


/**************************************************
               TblLastedMood
**************************************************/
const unsigned int TblLastedMoodID = 0x0048;
const unsigned int TblLastedMood_uin = 0x00480001;  //int32
const unsigned int TblLastedMood_MoodId = 0x00480002;  //int32
const unsigned int TblLastedMood_PostTime = 0x00480003;  //int32


/**************************************************
               TblGrpTypeName
**************************************************/
const unsigned int TblGrpTypeNameID = 0x0049;
const unsigned int TblGrpTypeName_UIN = 0x00490001;  //int32
const unsigned int TblGrpTypeName_TimeStamp = 0x00490002;  //int32
const unsigned int TblGrpTypeName_ClassName = 0x00490003;  //string128


/**************************************************
               TblActLog
**************************************************/
const unsigned int TblActLogID = 0x004a;
const unsigned int TblActLog_LogID = 0x004a0001;  //int32
const unsigned int TblActLog_ActID = 0x004a0002;  //int32
const unsigned int TblActLog_UIN = 0x004a0003;  //int32
const unsigned int TblActLog_ActTime = 0x004a0004;  //int32
const unsigned int TblActLog_ActSource = 0x004a0005;  //int8
const unsigned int TblActLog_CIP = 0x004a0006;  //int32
const unsigned int TblActLog_SIP = 0x004a0007;  //int32
const unsigned int TblActLog_content = 0x004a0008;  //binary


/**************************************************
               TblActIDCfg
**************************************************/
const unsigned int TblActIDCfgID = 0x004b;
const unsigned int TblActIDCfg_ActID = 0x004b0001;  //int32
const unsigned int TblActIDCfg_InsertTime = 0x004b0002;  //int32
const unsigned int TblActIDCfg_Remark = 0x004b0003;  //string256


/**************************************************
               TblActKeyIDCfg
**************************************************/
const unsigned int TblActKeyIDCfgID = 0x004c;
const unsigned int TblActKeyIDCfg_FieldID = 0x004c0001;  //int32
const unsigned int TblActKeyIDCfg_ActID = 0x004c0002;  //int32
const unsigned int TblActKeyIDCfg_Fieldtype = 0x004c0003;  //int8
const unsigned int TblActKeyIDCfg_Remark = 0x004c0004;  //string256


/**************************************************
               TblGrpHotKeywords
**************************************************/
const unsigned int TblGrpHotKeywordsID = 0x004d;
const unsigned int TblGrpHotKeywords_HotKeyWord = 0x004d0001;  //string128
const unsigned int TblGrpHotKeywords_HotCount = 0x004d0002;  //int32


/**************************************************
               TblGrpActive
**************************************************/
const unsigned int TblGrpActiveID = 0x004e;
const unsigned int TblGrpActive_ActiveID = 0x004e0001;  //int32
const unsigned int TblGrpActive_GroupUIN = 0x004e0002;  //int32
const unsigned int TblGrpActive_UserUIN = 0x004e0003;  //int32
const unsigned int TblGrpActive_Time = 0x004e0004;  //int32
const unsigned int TblGrpActive_Content = 0x004e0005;  //string512
const unsigned int TblGrpActive_type = 0x004e0006;  //int32


/**************************************************
               TblGrpNote
**************************************************/
const unsigned int TblGrpNoteID = 0x004f;
const unsigned int TblGrpNote_NoteID = 0x004f0001;  //int32
const unsigned int TblGrpNote_GroupUIN = 0x004f0002;  //int32
const unsigned int TblGrpNote_UserUIN = 0x004f0003;  //int32
const unsigned int TblGrpNote_Time = 0x004f0004;  //int32
const unsigned int TblGrpNote_Content = 0x004f0005;  //string512
const unsigned int TblGrpNote_ParentID = 0x004f0006;  //int32


/**************************************************
               TblGrpRecommend
**************************************************/
const unsigned int TblGrpRecommendID = 0x0050;
const unsigned int TblGrpRecommend_UserUIN = 0x00500001;  //int32
const unsigned int TblGrpRecommend_GroupUIN = 0x00500002;  //int32
const unsigned int TblGrpRecommend_Time = 0x00500003;  //int32
const unsigned int TblGrpRecommend_Flag = 0x00500004;  //int32


/**************************************************
               TblGrpVisitor
**************************************************/
const unsigned int TblGrpVisitorID = 0x0051;
const unsigned int TblGrpVisitor_GroupUIN = 0x00510001;  //int32
const unsigned int TblGrpVisitor_VisitorUIN = 0x00510002;  //int32
const unsigned int TblGrpVisitor_Time = 0x00510003;  //int32
const unsigned int TblGrpVisitor_Flag = 0x00510004;  //int32


/**************************************************
               TblGrpFriendGroup
**************************************************/
const unsigned int TblGrpFriendGroupID = 0x0052;
const unsigned int TblGrpFriendGroup_UIN = 0x00520001;  //int32
const unsigned int TblGrpFriendGroup_FUIN = 0x00520002;  //int32
const unsigned int TblGrpFriendGroup_GroupUIN = 0x00520003;  //int32
const unsigned int TblGrpFriendGroup_CreateTime = 0x00520004;  //int32


/**************************************************
               TblLoginHis
**************************************************/
const unsigned int TblLoginHisID = 0x0053;
const unsigned int TblLoginHis_UIN = 0x00530001;  //int32
const unsigned int TblLoginHis_Account = 0x00530002;  //string64
const unsigned int TblLoginHis_IP = 0x00530003;  //int32
const unsigned int TblLoginHis_SrcType = 0x00530004;  //int8
const unsigned int TblLoginHis_LoginType = 0x00530005;  //int8
const unsigned int TblLoginHis_Status = 0x00530006;  //int32
const unsigned int TblLoginHis_Time = 0x00530007;  //int32
const unsigned int TblLoginHis_OnlineTime = 0x00530008;  //int32


/**************************************************
               TblLoginInfo
**************************************************/
const unsigned int TblLoginInfoID = 0x0054;
const unsigned int TblLoginInfo_UIN = 0x00540001;  //int32
const unsigned int TblLoginInfo_LastLoginTime = 0x00540002;  //int32
const unsigned int TblLoginInfo_LastLoginIP = 0x00540003;  //int32
const unsigned int TblLoginInfo_LastLoginVer = 0x00540004;  //int32


/**************************************************
               TblAlarmRecvSetting
**************************************************/
const unsigned int TblAlarmRecvSettingID = 0x0055;
const unsigned int TblAlarmRecvSetting_UIN = 0x00550001;  //int32
const unsigned int TblAlarmRecvSetting_NeedPrompt = 0x00550002;  //int8
const unsigned int TblAlarmRecvSetting_Email = 0x00550003;  //string128
const unsigned int TblAlarmRecvSetting_Mphone = 0x00550004;  //string32
const unsigned int TblAlarmRecvSetting_ActivateEmail = 0x00550005;  //string128
const unsigned int TblAlarmRecvSetting_EmailVCode = 0x00550006;  //string32
const unsigned int TblAlarmRecvSetting_EmailVCodeTime = 0x00550007;  //int32
const unsigned int TblAlarmRecvSetting_ActivateMPhone = 0x00550008;  //string128
const unsigned int TblAlarmRecvSetting_MPhoneVCode = 0x00550009;  //string32
const unsigned int TblAlarmRecvSetting_MPhoneVCodeTime = 0x0055000a;  //int32


/**************************************************
               TblAlarmCondSetting
**************************************************/
const unsigned int TblAlarmCondSettingID = 0x0056;
const unsigned int TblAlarmCondSetting_UIN = 0x00560001;  //int32
const unsigned int TblAlarmCondSetting_StockIndex = 0x00560002;  //int16
const unsigned int TblAlarmCondSetting_AlarmSetting = 0x00560003;  //binary
const unsigned int TblAlarmCondSetting_LastModTime = 0x00560004;  //int32


/**************************************************
               TblNewsCount
**************************************************/
const unsigned int TblNewsCountID = 0x0057;
const unsigned int TblNewsCount_UIN = 0x00570001;  //int32
const unsigned int TblNewsCount_Type = 0x00570002;  //int32
const unsigned int TblNewsCount_Num = 0x00570003;  //int32


/**************************************************
               TblMsgInbox
**************************************************/
const unsigned int TblMsgInboxID = 0x0058;
const unsigned int TblMsgInbox_MsgID = 0x00580001;  //int32
const unsigned int TblMsgInbox_UIN = 0x00580002;  //int32
const unsigned int TblMsgInbox_FromUIN = 0x00580003;  //int32
const unsigned int TblMsgInbox_ParentID = 0x00580004;  //int32
const unsigned int TblMsgInbox_Content = 0x00580005;  //string1024
const unsigned int TblMsgInbox_RelateID = 0x00580006;  //int32
const unsigned int TblMsgInbox_Type = 0x00580007;  //int8
const unsigned int TblMsgInbox_SubType = 0x00580008;  //int8
const unsigned int TblMsgInbox_PostTime = 0x00580009;  //int64
const unsigned int TblMsgInbox_IsRead = 0x0058000a;  //int8
const unsigned int TblMsgInbox_IsDone = 0x0058000b;  //int8


/**************************************************
               TblMsgInnMail
**************************************************/
const unsigned int TblMsgInnMailID = 0x0059;
const unsigned int TblMsgInnMail_MailID = 0x00590001;  //int32
const unsigned int TblMsgInnMail_UIN = 0x00590002;  //int32
const unsigned int TblMsgInnMail_PeerUIN = 0x00590003;  //int32
const unsigned int TblMsgInnMail_Type = 0x00590004;  //int8
const unsigned int TblMsgInnMail_Content = 0x00590005;  //string800
const unsigned int TblMsgInnMail_PostTime = 0x00590006;  //int64
const unsigned int TblMsgInnMail_IsRead = 0x00590007;  //int8
const unsigned int TblMsgInnMail_UplevelID = 0x00590008;  //int8


/**************************************************
               TblGrpTransferHis
**************************************************/
const unsigned int TblGrpTransferHisID = 0x005a;
const unsigned int TblGrpTransferHis_FromUIN = 0x005a0001;  //int32
const unsigned int TblGrpTransferHis_ToUIN = 0x005a0002;  //int32
const unsigned int TblGrpTransferHis_GroupUIN = 0x005a0003;  //int32
const unsigned int TblGrpTransferHis_TransferTime = 0x005a0004;  //int32


/**************************************************
               TblFileInfo
**************************************************/
const unsigned int TblFileInfoID = 0x005b;
const unsigned int TblFileInfo_FileID = 0x005b0001;  //int32
const unsigned int TblFileInfo_FileHashCode = 0x005b0002;  //string128
const unsigned int TblFileInfo_FileSize = 0x005b0003;  //int32
const unsigned int TblFileInfo_NodeID = 0x005b0004;  //int32
const unsigned int TblFileInfo_FromUIN = 0x005b0005;  //int32
const unsigned int TblFileInfo_GroupType = 0x005b0006;  //int8
const unsigned int TblFileInfo_GroupUIN = 0x005b0007;  //int32
const unsigned int TblFileInfo_UploadTime = 0x005b0008;  //int32


/**************************************************
               TblFileSvrList
**************************************************/
const unsigned int TblFileSvrListID = 0x005c;
const unsigned int TblFileSvrList_NodeID = 0x005c0001;  //int32
const unsigned int TblFileSvrList_ServerIP = 0x005c0002;  //int32
const unsigned int TblFileSvrList_ServerPort = 0x005c0003;  //int32
const unsigned int TblFileSvrList_DiskSize = 0x005c0004;  //int32
const unsigned int TblFileSvrList_UsedSize = 0x005c0005;  //int32


/**************************************************
               TblStkBoardInfo
**************************************************/
const unsigned int TblStkBoardInfoID = 0x005d;
const unsigned int TblStkBoardInfo_BoardID = 0x005d0001;  //string16
const unsigned int TblStkBoardInfo_BoardName = 0x005d0002;  //string64
const unsigned int TblStkBoardInfo_BoardPID = 0x005d0003;  //string16
const unsigned int TblStkBoardInfo_BoardIndex = 0x005d0004;  //int32


/**************************************************
               TblFreeFileID
**************************************************/
const unsigned int TblFreeFileIDID = 0x005e;
const unsigned int TblFreeFileID_FileID = 0x005e0001;  //int32


/**************************************************
               TblRecycleFileID
**************************************************/
const unsigned int TblRecycleFileIDID = 0x005f;
const unsigned int TblRecycleFileID_FileID = 0x005f0001;  //int32


/**************************************************
               TblSysCfg
**************************************************/
const unsigned int TblSysCfgID = 0x0060;
const unsigned int TblSysCfg_CfgID = 0x00600001;  //int32
const unsigned int TblSysCfg_Section = 0x00600002;  //string50
const unsigned int TblSysCfg_CfgName = 0x00600003;  //string50
const unsigned int TblSysCfg_CfgValue = 0x00600004;  //string2000
const unsigned int TblSysCfg_Remark = 0x00600005;  //string200


/**************************************************
               TblGrpBBSPost
**************************************************/
const unsigned int TblGrpBBSPostID = 0x0061;
const unsigned int TblGrpBBSPost_PostID = 0x00610001;  //int64
const unsigned int TblGrpBBSPost_GroupUIN = 0x00610002;  //int32
const unsigned int TblGrpBBSPost_PosterUIN = 0x00610003;  //int32
const unsigned int TblGrpBBSPost_Title = 0x00610004;  //string128
const unsigned int TblGrpBBSPost_Content = 0x00610005;  //text
const unsigned int TblGrpBBSPost_PostTime = 0x00610006;  //int32
const unsigned int TblGrpBBSPost_ViewNum = 0x00610007;  //int32
const unsigned int TblGrpBBSPost_ReplyNum = 0x00610008;  //int32
const unsigned int TblGrpBBSPost_LastReplyTime = 0x00610009;  //int32
const unsigned int TblGrpBBSPost_IsElite = 0x0061000a;  //int32
const unsigned int TblGrpBBSPost_Status = 0x0061000b;  //int32


/**************************************************
               TblGrpBBSReply
**************************************************/
const unsigned int TblGrpBBSReplyID = 0x0062;
const unsigned int TblGrpBBSReply_ReplyID = 0x00620001;  //int64
const unsigned int TblGrpBBSReply_PostID = 0x00620002;  //int32
const unsigned int TblGrpBBSReply_ReplyUIN = 0x00620003;  //int32
const unsigned int TblGrpBBSReply_Content = 0x00620004;  //text
const unsigned int TblGrpBBSReply_ReplyTime = 0x00620005;  //int32


/**************************************************
               TblSequence
**************************************************/
const unsigned int TblSequenceID = 0x0063;
const unsigned int TblSequence_TypeID = 0x00630001;  //int32
const unsigned int TblSequence_Value = 0x00630002;  //int32


/**************************************************
               TblUserLevel
**************************************************/
const unsigned int TblUserLevelID = 0x0064;
const unsigned int TblUserLevel_UserUIN = 0x00640001;  //int32
const unsigned int TblUserLevel_Experience = 0x00640002;  //int32
const unsigned int TblUserLevel_Flag = 0x00640003;  //int32


/**************************************************
               TblBannerCfg
**************************************************/
const unsigned int TblBannerCfgID = 0x0065;
const unsigned int TblBannerCfg_ID = 0x00650001;  //int32
const unsigned int TblBannerCfg_FileUrl = 0x00650002;  //string256
const unsigned int TblBannerCfg_FileHash = 0x00650003;  //string256
const unsigned int TblBannerCfg_LinkUrl = 0x00650004;  //string256
const unsigned int TblBannerCfg_DateStart = 0x00650005;  //int32
const unsigned int TblBannerCfg_EndStart = 0x00650006;  //int32
const unsigned int TblBannerCfg_TimeOfDateStart = 0x00650007;  //int16
const unsigned int TblBannerCfg_TimeOfDateEnd = 0x00650008;  //int16
const unsigned int TblBannerCfg_PriorityType = 0x00650009;  //int8
const unsigned int TblBannerCfg_PriorityValue = 0x0065000a;  //int8
const unsigned int TblBannerCfg_CfgVer = 0x0065000b;  //int32
const unsigned int TblBannerCfg_Remark = 0x0065000c;  //string200


/**************************************************
               TblMobileBind
**************************************************/
const unsigned int TblMobileBind = 0x0066;
const unsigned int TblMObileBind_UIN = 0x00660001;  //int32
const unsigned int TblMObileBind_Mobile1 = 0x00660002;  //string20
const unsigned int TblMObileBind_Mask1 = 0x00660003;  //int32
const unsigned int TblMObileBind_Mobile2 = 0x00660004;  //string20
const unsigned int TblMObileBind_Mask2 = 0x00660005;  //int32
const unsigned int TblMObileBind_Mobile3 = 0x00660006;  //string20
const unsigned int TblMObileBind_Mask3 = 0x00660007;  //int32
const unsigned int TblMObileBind_Mobile4 = 0x00660008;  //string20
const unsigned int TblMObileBind_Mask4 = 0x00660009;  //int32
const unsigned int TblMObileBind_Mobile5 = 0x0066000a;  //string20
const unsigned int TblMObileBind_Mask5 = 0x0066000b;  //int32
const unsigned int TblMObileBind_Mobile6 = 0x0066000c;  //string20
const unsigned int TblMObileBind_Mask6 = 0x0066000d;  //int32


/**************************************************
               TblMblogUser
**************************************************/
const unsigned int TblMblogUser = 0x0067;
const unsigned int TblMblogUser_UIN = 0x00670001;  //int32
const unsigned int TblMblogUser_PostNum = 0x00670002;  //int32
const unsigned int TblMblogUser_FollowedNum = 0x00670003;  //int32
const unsigned int TblMblogUser_Description = 0x00670004;  //string512


/**************************************************
               TblMblogHistory
**************************************************/
const unsigned int TblMblogHistory = 0x0068;
const unsigned int TblMblogHistory_ItemId = 0x00680001;  //int64
const unsigned int TblMblogHistory_UIN = 0x00680002;  //int32
const unsigned int TblMblogHistory_Content = 0x00680003; //string512
const unsigned int TblMblogHistory_PostTime = 0x00680004; //int32
const unsigned int TblMblogHistory_OrigId = 0x00680005;  //int64
const unsigned int TblMblogHistory_OrigUIN = 0x00680006;  //int32
const unsigned int TblMblogHistory_Status = 0x00680007;  //int32
const unsigned int TblMblogHistory_TransmitNum = 0x00680008;  //int32
const unsigned int TblMblogHistory_ItemType = 0x00680009;  //int32
const unsigned int TblMblogHistory_InfoStr = 0x0068000a;  //string1024


/**************************************************
               TblMblogCelebrity
**************************************************/
const unsigned int TblMblogCelebrity = 0x0069;
const unsigned int TblMblogCelebrity_UIN = 0x00690001;  //int32
const unsigned int TblMblogCelebrity_Status = 0x00690002;  //int32
const unsigned int TblMblogCelebrity_Level = 0x00690003;  //int32
const unsigned int TblMblogCelebrity_Description = 0x00690004;  //string512


/**************************************************
               TblMblogMyFollow
**************************************************/
const unsigned int TblMblogMyFollow = 0x006a;
const unsigned int TblMblogMyFollow_UIN = 0x006a0001;  //int32
const unsigned int TblMblogMyFollow_FollowUIN = 0x006a0002;  //int32
const unsigned int TblMblogMyFollow_FollowTime = 0x006a0003;  //int32


/**************************************************
               TblMblogFollowed
**************************************************/
const unsigned int TblMblogFollowed = 0x006b;
const unsigned int TblMblogFollowed_UIN = 0x006b0001;  //int32
const unsigned int TblMblogFollowed_FollowUIN = 0x006b0002;  //int32
const unsigned int TblMblogFollowed_FollowTime = 0x006b0003;  //int32


/**************************************************
               TblMblogStocks
**************************************************/
const unsigned int TblMblogStocks = 0x006c;
const unsigned int TblMblogStocks_StockID = 0x006c0001;  //int32
const unsigned int TblMblogStocks_ItemID = 0x006c0002;  //int64
const unsigned int TblMblogStocks_UIN = 0x006c0003;  //int32
const unsigned int TblMblogStocks_PostTime = 0x006c0004;  //int32


/**************************************************
               TblMblogTopic
**************************************************/
const unsigned int TblMblogTopic = 0x006d;
const unsigned int TblMblogTopic_TopicID = 0x006d0001;  //int32
const unsigned int TblMblogTopic_TopicName = 0x006d0002;  //string512
const unsigned int TblMblogTopic_Status = 0x006d0003;  //int32
const unsigned int TblMblogTopic_PostTime = 0x006d0004;  //int32


/**************************************************
               TblMblogTopicContent
**************************************************/
const unsigned int TblMblogTopicContent = 0x006e;
const unsigned int TblMblogTopicContent_TopicID = 0x006e0001;  //int32
const unsigned int TblMblogTopicContent_ItemID = 0x006e0002;  //int64
const unsigned int TblMblogTopicContent_UIN = 0x006e0003;  //int32
const unsigned int TblMblogTopicContent_PostTime = 0x006e0004;  //int32


/**************************************************
               TblNewFamiliar
**************************************************/
const unsigned int TblNewFamiliar = 0x006f;
const unsigned int TblNewFamiliar_UIN = 0x006f0001;  //int32
const unsigned int TblNewFamiliar_FamiliarInfo = 0x006f0002;  //binary


/**************************************************
               TblFeedback
**************************************************/
const unsigned int TblFeedback = 0x0070;
const unsigned int TblFeedback_FeedId = 0x00700001;  //int32
const unsigned int TblFeedback_Title = 0x00700002;  //string512
const unsigned int TblFeedback_Content = 0x00700003;  //text
const unsigned int TblFeedback_Description = 0x00700004;  //string512
const unsigned int TblFeedback_Contact = 0x00700005;  //string256
const unsigned int TblFeedback_PostTime = 0x00700006;  //int32
const unsigned int TblFeedback_Status = 0x00700007;  //int32


/**************************************************
               TblStkNowData
**************************************************/
const unsigned int TblStkNowDataID                          = 0x0071;

const unsigned int TblStkNowData_StockIndex                 = 0x00710001;    //  int32    primary
const unsigned int TblStkNowData_DateTimeStamp              = 0x00710002;    //  int32    primary
const unsigned int TblStkNowData_StockCode                  = 0x00710003;    //  string8
const unsigned int TblStkNowData_MarketType                 = 0x00710004;    //  int32
const unsigned int TblStkNowData_NowValue                   = 0x00710005;    //  int32
const unsigned int TblStkNowData_PrevClose                  = 0x00710006;    //  int32
const unsigned int TblStkNowData_TodayOpen                  = 0x00710007;    //  int32
const unsigned int TblStkNowData_TodayHigh                  = 0x00710008;    //  int32
const unsigned int TblStkNowData_TodayLow                   = 0x00710009;    //  int32
const unsigned int TblStkNowData_Amount                     = 0x0071000a;    //  int64
const unsigned int TblStkNowData_Volume                     = 0x0071000b;    //  int64
const unsigned int TblStkNowData_AvePrice                   = 0x0071000c;    //  int32
const unsigned int TblStkNowData_UpSpeed                    = 0x0071000d;    //  int32
const unsigned int TblStkNowData_Liangbi                    = 0x0071000e;    //  int32
const unsigned int TblStkNowData_TotalBuyVol                = 0x0071000f;    //  int32
const unsigned int TblStkNowData_TotalSellVol               = 0x00710010;    //  int32
const unsigned int TblStkNowData_BuyCount                   = 0x00710011;    //  int32
const unsigned int TblStkNowData_SellCount                  = 0x00710012;    //  int32
const unsigned int TblStkNowData_BuyPrice1                  = 0x00710013;    //  int32
const unsigned int TblStkNowData_BuyVol1                    = 0x00710014;    //  int32
const unsigned int TblStkNowData_BuyPrice2                  = 0x00710015;    //  int32
const unsigned int TblStkNowData_BuyVol2                    = 0x00710016;    //  int32
const unsigned int TblStkNowData_BuyPrice3                  = 0x00710017;    //  int32
const unsigned int TblStkNowData_BuyVol3                    = 0x00710018;    //  int32
const unsigned int TblStkNowData_BuyPrice4                  = 0x00710019;    //  int32
const unsigned int TblStkNowData_BuyVol4                    = 0x0071001a;    //  int32
const unsigned int TblStkNowData_BuyPrice5                  = 0x0071001b;    //  int32
const unsigned int TblStkNowData_BuyVol5                    = 0x0071001c;    //  int32
const unsigned int TblStkNowData_BuyPrice6                  = 0x0071001d;    //  int32
const unsigned int TblStkNowData_BuyVol6                    = 0x0071001e;    //  int32
const unsigned int TblStkNowData_BuyPrice7                  = 0x0071001f;    //  int32
const unsigned int TblStkNowData_BuyVol7                    = 0x00710020;    //  int32
const unsigned int TblStkNowData_BuyPrice8                  = 0x00710021;    //  int32
const unsigned int TblStkNowData_BuyVol8                    = 0x00710022;    //  int32
const unsigned int TblStkNowData_BuyPrice9                  = 0x00710023;    //  int32
const unsigned int TblStkNowData_BuyVol9                    = 0x00710024;    //  int32
const unsigned int TblStkNowData_BuyPrice10                 = 0x00710025;    //  int32
const unsigned int TblStkNowData_BuyVol10                   = 0x00710026;    //  int32
const unsigned int TblStkNowData_SellPrice1                 = 0x00710027;    //  int32
const unsigned int TblStkNowData_SellVol1                   = 0x00710028;    //  int32
const unsigned int TblStkNowData_SellPrice2                 = 0x00710029;    //  int32
const unsigned int TblStkNowData_SellVol2                   = 0x0071002a;    //  int32
const unsigned int TblStkNowData_SellPrice3                 = 0x0071002b;    //  int32
const unsigned int TblStkNowData_SellVol3                   = 0x0071002c;    //  int32
const unsigned int TblStkNowData_SellPrice4                 = 0x0071002d;    //  int32
const unsigned int TblStkNowData_SellVol4                   = 0x0071002e;    //  int32
const unsigned int TblStkNowData_SellPrice5                 = 0x0071002f;    //  int32
const unsigned int TblStkNowData_SellVol5                   = 0x00710030;    //  int32
const unsigned int TblStkNowData_SellPrice6                 = 0x00710031;    //  int32
const unsigned int TblStkNowData_SellVol6                   = 0x00710032;    //  int32
const unsigned int TblStkNowData_SellPrice7                 = 0x00710033;    //  int32
const unsigned int TblStkNowData_SellVol7                   = 0x00710034;    //  int32
const unsigned int TblStkNowData_SellPrice8                 = 0x00710035;    //  int32
const unsigned int TblStkNowData_SellVol8                   = 0x00710036;    //  int32
const unsigned int TblStkNowData_SellPrice9                 = 0x00710037;    //  int32
const unsigned int TblStkNowData_SellVol9                   = 0x00710038;    //  int32
const unsigned int TblStkNowData_SellPrice10                = 0x00710039;    //  int32
const unsigned int TblStkNowData_SellVol10                  = 0x0071003a;    //  int32
const unsigned int TblStkNowData_PE                         = 0x0071003b;    //  int32
const unsigned int TblStkNowData_PB                         = 0x0071003c;    //  int32


/**************************************************
               TblStkMinuteData
**************************************************/
const unsigned int TblStkMinuteDataID                       = 0x0072;

const unsigned int TblStkMinuteData_StockIndex              = 0x00720001;    //  int32    primary
const unsigned int TblStkMinuteData_DataTimeStamp           = 0x00720002;    //  int32    primary
const unsigned int TblStkMinuteData_StockCode               = 0x00720003;    //  string8
const unsigned int TblStkMinuteData_MarketType              = 0x00720004;    //  int32
const unsigned int TblStkMinuteData_NowValue                = 0x00720005;    //  int32
const unsigned int TblStkMinuteData_Amount                  = 0x00720006;    //  int64
const unsigned int TblStkMinuteData_Volume                  = 0x00720007;    //  int64


/**************************************************
               TblIndexMinuteData
**************************************************/
const unsigned int TblIndexMinuteDataID                     = 0x0073;

const unsigned int TblIndexMinuteData_StockIndex            = 0x00730001;    //  int32    primary
const unsigned int TblIndexMinuteData_DataTimeStamp         = 0x00730002;    //  int32    primary
const unsigned int TblIndexMinuteData_StockCode             = 0x00730003;    //  string8
const unsigned int TblIndexMinuteData_MarketType            = 0x00730004;    //  int32
const unsigned int TblIndexMinuteData_NowValue              = 0x00730005;    //  int32
const unsigned int TblIndexMinuteData_Amount                = 0x00730006;    //  int64
const unsigned int TblIndexMinuteData_Volume                = 0x00730007;    //  int64
const unsigned int TblIndexMinuteData_NoWeightedNowValue    = 0x00730008;    //  int32
const unsigned int TblIndexMinuteData_UpCount               = 0x00730009;    //  int32
const unsigned int TblIndexMinuteData_DownCount             = 0x0073000a;    //  int32


/**************************************************
               TblIndexNowData
**************************************************/
const unsigned int TblIndexNowDataID                        = 0x0074;

const unsigned int TblIndexNowData_StockIndex               = 0x00740001;    //  int32    primary
const unsigned int TblIndexNowData_DataTimeStamp            = 0x00740002;    //  int32    primary
const unsigned int TblIndexNowData_StockCode                = 0x00740003;    //  string8
const unsigned int TblIndexNowData_MarketType               = 0x00740004;    //  int32
const unsigned int TblIndexNowData_NowValue                 = 0x00740005;    //  int32
const unsigned int TblIndexNowData_PrevClose                = 0x00740006;    //  int32
const unsigned int TblIndexNowData_TodayOpen                = 0x00740007;    //  int32
const unsigned int TblIndexNowData_TodayHigh                = 0x00740008;    //  int32
const unsigned int TblIndexNowData_TodayLow                 = 0x00740009;    //  int32
const unsigned int TblIndexNowData_AvePrice                 = 0x0074000a;    //  int32
const unsigned int TblIndexNowData_Amount                   = 0x0074000b;    //  int64
const unsigned int TblIndexNowData_Volume                   = 0x0074000c;    //  int64


/**************************************************
               TblLastMblogInfo
**************************************************/
const unsigned int TblLastMblogInfoID                       = 0x0075;

const unsigned int TblLastMblogInfo_UIN                     = 0x00750001;    //  int32    primary
const unsigned int TblLastMblogInfo_ItemID                  = 0x00750002;    //  int64
const unsigned int TblLastMblogInfo_PostTime                = 0x00750003;    //  int32


/**************************************************
               TblRegHis
**************************************************/
const unsigned int TblRegHisID                              = 0x0076;

const unsigned int TblRegHis_Account                        = 0x00760001;    //  string64
const unsigned int TblRegHis_SrcType                        = 0x00760002;    //  int8
const unsigned int TblRegHis_OPType                         = 0x00760003;    //  int8
const unsigned int TblRegHis_Status                         = 0x00760004;    //  int32
const unsigned int TblRegHis_IP                             = 0x00760005;    //  int32
const unsigned int TblRegHis_Time                           = 0x00760006;    //  int32


/**************************************************
               TblFeeds
**************************************************/
const unsigned int TblFeedsID                               = 0x0077;

const unsigned int TblFeeds_ID                              = 0x00770001;    //  int32
const unsigned int TblFeeds_UIN                             = 0x00770002;    //  int32
const unsigned int TblFeeds_AppId                           = 0x00770003;    //  int32
const unsigned int TblFeeds_FeedsType                       = 0x00770004;    //  int16
const unsigned int TblFeeds_SubmitTime                      = 0x00770005;    //  int32
const unsigned int TblFeeds_Content                         = 0x00770006;    //  text


/**************************************************
               TblFeedsCfg
**************************************************/
const unsigned int TblFeedsCfgID                            = 0x0078;

const unsigned int TblFeedsCfg_Template_ID                  = 0x00780001;    //  int16
const unsigned int TblFeedsCfg_AppId                        = 0x00780002;    //  int32
const unsigned int TblFeedsCfg_Dest_Flag                    = 0x00780003;    //  int8
const unsigned int TblFeedsCfg_Dest_UIN_Key                 = 0x00780004;    //  string128
const unsigned int TblFeedsCfg_Dest_Name_Key                = 0x00780005;    //  string128
const unsigned int TblFeedsCfg_Template_Text                = 0x00780006;    //  text
const unsigned int TblFeedsCfg_Marker                       = 0x00780007;    //  string256


/**************************************************
               TblStkSort
**************************************************/
const unsigned int TblStkSortID                             = 0x0079;

const unsigned int TblStkSort_UIN                           = 0x00790001;    //  int32
const unsigned int TblStkSort_GrpID                         = 0x00790002;    //  int32
const unsigned int TblStkSort_SortContent                   = 0x00790003;    //  binary


/**************************************************
               TblStkRank
**************************************************/
const unsigned int TblStkRankID                             = 0x007a;

const unsigned int TblStkRank_Type                          = 0x007a0001;    //  int32
const unsigned int TblStkRank_RankContent                   = 0x007a0002;    //  binary
const unsigned int TblStkRank_Time                          = 0x007a0003;    //  int32


/**************************************************
               TblVoteInfo
**************************************************/
const unsigned int TblVoteInfoID                            = 0x007b;

const unsigned int TblVoteInfo_UIN                          = 0x007b0001;    //  int32    primary
const unsigned int TblVoteInfo_vid                          = 0x007b0002;    //  int32    primary
const unsigned int TblVoteInfo_PostTime                     = 0x007b0003;    //  int32
const unsigned int TblVoteInfo_EndTime                      = 0x007b0004;    //  int32
const unsigned int TblVoteInfo_OptType                      = 0x007b0005;    //  int8
const unsigned int TblVoteInfo_Status                       = 0x007b0006;    //  int8
const unsigned int TblVoteInfo_Rights                       = 0x007b0007;    //  int8
const unsigned int TblVoteInfo_TotalNum                     = 0x007b0008;    //  int32
const unsigned int TblVoteInfo_DayNum                       = 0x007b0009;    //  int32
const unsigned int TblVoteInfo_LastVoteTime                 = 0x007b000a;    //  int32
const unsigned int TblVoteInfo_ReplyNum                     = 0x007b000b;    //  int32
const unsigned int TblVoteInfo_MaxReplyId                   = 0x007b000c;    //  int32
const unsigned int TblVoteInfo_Subject                      = 0x007b000d;    //  string255
const unsigned int TblVoteInfo_SubjectDesc                  = 0x007b000e;    //  text
const unsigned int TblVoteInfo_Options                      = 0x007b000f;    //  text
const unsigned int TblVoteInfo_OptionsDesc                  = 0x007b0010;    //  text
const unsigned int TblVoteInfo_New                          = 0x007b0011;    //  text


/**************************************************
               TblVoteReply
**************************************************/
const unsigned int TblVoteReplyID                           = 0x007c;

const unsigned int TblVoteReply_UIN                         = 0x007c0001;    //  int32    primary
const unsigned int TblVoteReply_vid                         = 0x007c0002;    //  int32    primary
const unsigned int TblVoteReply_Rid                         = 0x007c0003;    //  int32    primary
const unsigned int TblVoteReply_ReplyUin                    = 0x007c0004;    //  int32
const unsigned int TblVoteReply_ReplyTime                   = 0x007c0005;    //  int32
const unsigned int TblVoteReply_Content                     = 0x007c0006;    //  text


/**************************************************
               TblVoteRecord
**************************************************/
const unsigned int TblVoteRecordID                          = 0x007d;

const unsigned int TblVoteRecord_UIN                        = 0x007d0001;    //  int32    primary
const unsigned int TblVoteRecord_Sponser                    = 0x007d0002;    //  int32    primary
const unsigned int TblVoteRecord_Vid                        = 0x007d0003;    //  int32    primary
const unsigned int TblVoteRecord_PostTime                   = 0x007d0004;    //  int32


/**************************************************
               TblFriendsVote
**************************************************/
const unsigned int TblFriendsVoteID                         = 0x007e;

const unsigned int TblFriendsVote_UIN                       = 0x007e0001;    //  int32    primary
const unsigned int TblFriendsVote_PostVote                  = 0x007e0002;    //  binary
const unsigned int TblFriendsVote_PartVote                  = 0x007e0003;    //  binary


/**************************************************
               TblVoteStat
**************************************************/
const unsigned int TblVoteStatID                            = 0x007f;

const unsigned int TblVoteStat_UIN                          = 0x007f0001;    //  int32    primary
const unsigned int TblVoteStat_vid                          = 0x007f0002;    //  int32    primary
const unsigned int TblVoteStat_Opt1                         = 0x007f0003;    //  int32
const unsigned int TblVoteStat_Opt2                         = 0x007f0004;    //  int32
const unsigned int TblVoteStat_Opt3                         = 0x007f0005;    //  int32
const unsigned int TblVoteStat_Opt4                         = 0x007f0006;    //  int32
const unsigned int TblVoteStat_Opt5                         = 0x007f0007;    //  int32
const unsigned int TblVoteStat_Opt6                         = 0x007f0008;    //  int32
const unsigned int TblVoteStat_Opt7                         = 0x007f0009;    //  int32
const unsigned int TblVoteStat_Opt8                         = 0x007f000a;    //  int32
const unsigned int TblVoteStat_Opt9                         = 0x007f000b;    //  int32
const unsigned int TblVoteStat_Opt10                        = 0x007f000c;    //  int32
const unsigned int TblVoteStat_Opt11                        = 0x007f000d;    //  int32
const unsigned int TblVoteStat_Opt12                        = 0x007f000e;    //  int32
const unsigned int TblVoteStat_Opt13                        = 0x007f000f;    //  int32
const unsigned int TblVoteStat_Opt14                        = 0x007f0010;    //  int32
const unsigned int TblVoteStat_Opt15                        = 0x007f0011;    //  int32
const unsigned int TblVoteStat_Opt16                        = 0x007f0012;    //  int32
const unsigned int TblVoteStat_Opt17                        = 0x007f0013;    //  int32
const unsigned int TblVoteStat_Opt18                        = 0x007f0014;    //  int32
const unsigned int TblVoteStat_Opt19                        = 0x007f0015;    //  int32
const unsigned int TblVoteStat_Opt20                        = 0x007f0016;    //  int32
const unsigned int TblVoteStat_Opt21                        = 0x007f0017;    //  int32
const unsigned int TblVoteStat_Opt22                        = 0x007f0018;    //  int32
const unsigned int TblVoteStat_Opt23                        = 0x007f0019;    //  int32
const unsigned int TblVoteStat_Opt24                        = 0x007f001a;    //  int32
const unsigned int TblVoteStat_Opt25                        = 0x007f001b;    //  int32
const unsigned int TblVoteStat_Opt26                        = 0x007f001c;    //  int32
const unsigned int TblVoteStat_Opt27                        = 0x007f001d;    //  int32
const unsigned int TblVoteStat_Opt28                        = 0x007f001e;    //  int32
const unsigned int TblVoteStat_Opt29                        = 0x007f001f;    //  int32
const unsigned int TblVoteStat_Opt30                        = 0x007f0020;    //  int32


/**************************************************
               TblStkUserHis
**************************************************/
const unsigned int TblStkUserHisID                          = 0x0080;

const unsigned int TblStkUserHis_UIN                        = 0x00800001;    //  int32
const unsigned int TblStkUserHis_SrcType                    = 0x00800002;    //  int8
const unsigned int TblStkUserHis_StkIdx                     = 0x00800003;    //  int32
const unsigned int TblStkUserHis_Type                       = 0x00800004;    //  int8
const unsigned int TblStkUserHis_Time                       = 0x00800005;    //  int32


/**************************************************
               TblStkStat
**************************************************/
const unsigned int TblStkStatID                             = 0x0081;

const unsigned int TblStkStat_StkIdx                        = 0x00810001;    //  int32    primary
const unsigned int TblStkStat_Type                          = 0x00810002;    //  int32    primary
const unsigned int TblStkStat_Time                          = 0x00810003;    //  int32    primary
const unsigned int TblStkStat_Num                           = 0x00810004;    //  int32


/**************************************************
               TblMobileArea
**************************************************/
const unsigned int TblMobileAreaID                             = 0x0082;

const unsigned int TblMobileArea_MobileNumber                  = 0x00820001;    //  int32    primary
const unsigned int TblMobileArea_MobileArea                    = 0x00820002;    //  string30
const unsigned int TblMobileArea_Province                      = 0x00820003;    //  string50
const unsigned int TblMobileArea_City                          = 0x00820004;    //  string50
const unsigned int TblMobileArea_AreaCode                      = 0x00820005;    //  string30


#endif // _JCACHE_FIELDID_H_
