<?php
include_once(APPPATH."core/TT_Model.php");
class Grouprelation_model extends TT_Model{
	function __construct(){
		parent::__construct();
		$this->table_name = 'IMGroupMember';
	}

}