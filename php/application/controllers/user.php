<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

include_once(APPPATH."core/TT_Controller.php");

class User extends TT_Controller {

	public function __construct()
	{
		parent::__construct();
		$this->load->helper('url');
		$this->load->model('user_model');
		$this->load->model('depart_model');
	}

	public function index()
	{
		$this->config->site_url();
		$this->load->view('base/header');
		$this->load->view('base/user');
		$this->load->view('base/footer');
	}

	public function all()
	{
		$perpage = 10000;
		$departs = $this->depart_model->getList(array('status'=>0), '*', 0, $perpage);
		$_departs = array();
		foreach ($departs as $key => $value) {
			$_departs[$value['id']] = $value;
		}
		$start = $this->input->get('start');
		if(!$start){
			$start =  0;
		}
		$perpage = 10;
		$users = $this->user_model->getList(array('status'=>0), '*', $start*$perpage, $perpage);
		foreach ($users as $key => $value) {
			if($value['sex'] == 0){
				$users[$key]['sex'] = '女';
			}else{
				$users[$key]['sex'] = '男';
			}
			if(isset($_departs[$value['departId']])){
				$users[$key]['depart_value'] = $_departs[$value['departId']]['departName'];
			}else{
				$users[$key]['depart_value'] = '数据错误';
			}
			if($users[$key]['avatar']){
				$users[$key]['avatar_value'] = $users[$key]['avatar'];
			}
		}
		$count = $this->user_model->getCount(array('status'=>0));
		$result = array(
			'users'=>$users,
			'page'=>$start,
			'count'=>ceil($count/10),
			'departs'=>$_departs
		);
		echo json_encode($result);
	}

	public function del()
	{
		$id = $this->input->post('id');
		$result = $this->user_model->update(array('status'=>3), $id);
		if($result){
			echo 'success';
		}
	}

	public function add()
	{
		$salt = rand()%10000;
		$params = array(
			'sex'=>$this->input->post('sex'),
			'name'=>$this->input->post('name'),
			'domain'=>$this->input->post('domain'),
			'nick'=>$this->input->post('nick'),
			'password'=>md5(md5($this->input->post('password')).$salt),
			'salt'=>$salt,
			'phone'=>$this->input->post('phone'),
			'email'=>$this->input->post('email'),
			'avatar'=>$this->input->post('avatar'),
			'departId'=>$this->input->post('departId'),
			'status'=>$this->input->post('status'),
			'created'=>time(),
			'updated'=>time()
		);
		$result = $this->user_model->insert($params);
		if($result){
			echo 'success';
		}
	}

	public function edit()
	{
		$avatar = $this->input->post('avatar');
		if(strpos($avatar, $this->config->config['msfs_url']) === false){
			$avatar = $this->config->config['msfs_url'].$this->input->post('avatar');
		}
		$params = array(
			'sex'=>$this->input->post('sex'),
			'name'=>$this->input->post('name'),
			'domain'=>$this->input->post('domain'),
			'nick'=>$this->input->post('nick'),
			'phone'=>$this->input->post('phone'),
			'email'=>$this->input->post('email'),
			'avatar'=>$avatar,
			'departId'=>$this->input->post('departId'),
			'status'=>$this->input->post('status'),
			'updated'=>time()
		);
		$id = $this->input->post('id');
		$result = $this->user_model->getOne(array('id'=>$id));
		$pwd = $this->input->post('pwd');
		if($pwd){
			$params['pwd'] = md5(md5($this->input->post('password')).$result["salt"]);
		}
		$result = $this->user_model->update($params,$id);
		if($result){
			echo 'success';
		}
	}

	public function get()
	{
		$id = $this->input->post('id');
		$result = $this->user_model->getOne(array('id'=>$id));
		if($result){
			echo json_encode($result);
		}
	}

	public function upload()
	{
		include_once APPPATH."libraries/image_moo.php";

		try{
		    $filename=$this->input->get('filename');
		    $ext = pathinfo($filename, PATHINFO_EXTENSION);
		    $filename = time().".".$ext;
		    $input = file_get_contents("php://input");
		    file_put_contents('./download/'.$filename, $input);
		    // $image = new Image_moo();
		    // $image
		    // 	->load('./download/'.$filename)
		    // 	->resize_crop(100,100)
		    // 	->save('./download/1.jpg');
		    // 裁剪头像
		    // $targ_w = $targ_h = 100;
		    // $jpeg_quality = 90;

		    // $img_r = imagecreatefromjpeg('./download/'.$filename);
		    // $dst_r = ImageCreateTrueColor( $targ_w, $targ_h );

		    // imagecopyresampled($dst_r,$img_r,0,0,0,0,
		    //     $targ_w,$targ_h,100,100);

		    // header('Content-type: image/jpeg');
		    // imagejpeg($dst_r, './download/'.$filename, $jpeg_quality);



		    $res = $this->_upload('./download/'.$filename);
		    if($res['error_code'] == 0){	    	
			    $array = array(
			    	'status' =>'success',
			    	'file' =>$res['path'],
			    	'real_path'=>$this->config->config['msfs_url'].$res['path']
			    );
		    }else{
		    	$array = array(
			    	'status' =>'fail',
			    	'file' =>'',
			    	'real_path'=>''
			    );
		    }
			echo json_encode($array);
		}
		catch(Exception $e)
		{
			$array = array(
				'status' =>'fail',
				'file' =>0
			);
			echo json_encode($array);
		}
	}

	public function _upload($filename)
	{
		$ch = curl_init();
		$data = array('filename'=>'@'.$filename);
		curl_setopt($ch,CURLOPT_URL,$this->config->config['msfs_url']);
		curl_setopt($ch,CURLOPT_RETURNTRANSFER,true);
		curl_setopt($ch,CURLOPT_POST,true);
		curl_setopt($ch,CURLOPT_POSTFIELDS,$data);
		$result = curl_exec($ch);
		curl_close($ch);
		return json_decode($result,1);
	}

}
