<script src="/ui/js/jquery.Jcrop.js"></script>
<link rel="stylesheet" href="/ui/css/jquery.Jcrop.css" type="text/css" />
<aside class="right-side">
    <section class="content-header">
        <h1>
            用户管理
            <small>欢迎来到TeamTalk</small>
        </h1>
        <ol class="breadcrumb">
            <li><a href="/"><i class="fa fa-dashboard"></i> 首页</a></li>
            <li class="active">用户管理</li>
        </ol>
    </section>
    <section class="content">
        <button style="margin-bottom:10px;" class="btn btn-primary btn-sm add_user pull-right">新增</button>
        <table class="table table-bordered">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>性别</th>
                    <th>用户名</th>
                    <th>花名</th>
                    <th>手机</th>
                    <th>email</th>
                    <th>头像</th>
                    <th>部门</th>
                    <th>编辑</th>
                </tr>
            </thead>
            <tbody>
            </tbody>
        </table>
        <ul class="pagination">
            <li class="prev_page">
                <a href="javascript:;">上一页</a>
            </li>
            <li class="show_page">
                <a href="javascript:;"></a>
            </li>
            <li class="next_page">
                <a href="javascript:;">下一页</a>
            </li>
        </ul>
    </section>
    <script>
        var User = {
            compiledTpl : null,
            compiledAddTpl : null,
            getUser : function(page){
                if(!page){
                    page = 0;
                }
                $.getJSON('/user/all', {
                    start : page
                }, function(data) {
                    User.tpl();
                    var _tpl = User.compiledTpl.render(data);
                    $("table").data('departs',data.departs);
                    $("tbody").html(_tpl);
                    if(data.page == 0){
                        $(".pagination .prev_page").addClass("disabled");   
                    }else{
                        $(".pagination .prev_page").removeClass("disabled").data('page',data.page-0-1);   
                    }
                    if(data.page == (data.count-1)){
                        $(".pagination .next_page").addClass("disabled");   
                    }else{
                        $(".pagination .next_page").removeClass("disabled").data('page',data.page-0+1);   
                    }
                    $(".pagination .show_page a").text('共'+data.count+'页');
                });
            },
            delUser : function(node){
                $.post('/user/del', {id: node.data('id')}, function(data) {
                    if($.trim(data) == 'success'){
                        node.fadeOut();
                    }
                });
            },
            editUser : function(){
                $.post('/user/edit', {
                    id:$(".btn-margin-edit").data('id'),
                    sex: $(".sex:checked").val(),
                    name: $(".name").val(),
                    nick: $(".nick").val(),
                    password: $(".password").val(),
                    phone: $(".phone").val(),
                    email: $(".email").val(),
                    avatar: $(".avatar").val(),
                    departId: $(".departId").val(),
                    crop_info:$(".crop_info").val()
                }, function(data) {
                    if($.trim(data) == 'success'){
                        $.fn.hideModal();
                        $(".add_user").before('<div class="alert alert-success alert-dismissable"><i class="fa fa-check"></i><button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>编辑成功</div>')
                        setTimeout(function(){
                            $(".alert").remove();
                        },3000);
                        User.getUser();
                    }else{
                        $(".btn-margin-edit").text('编辑失败');
                    }
                });
            },
            addUser : function(){
                $.post('/user/add', {
                    sex: $(".sex:checked").val(),
                    name: $(".name").val(),
                    nick: $(".nick").val(),
                    password: $(".password").val(),
                    phone: $(".phone").val(),
                    email: $(".email").val(),
                    avatar: $(".avatar").val(),
                    departId: $(".departId").val()
                }, function(data) {
                    if($.trim(data) == 'success'){
                        $.fn.hideModal();
                        $(".add_user").before('<div class="alert alert-success alert-dismissable"><i class="fa fa-check"></i><button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>添加成功</div>')
                        setTimeout(function(){
                            $(".alert").remove();
                        },3000);
                    }else{
                        $(".btn-margin").text('添加失败');
                    }
                });
            },
            editUserAlert : function(node){
                User.addTpl();
                $.fn.SimpleModal({
                    btn_ok: '编辑',
                    model: 'confirm',
                    callback: function(node){
                        User.editUser();
                    },
                    overlayClick: false,
                    width: 660,
                    title: '编辑用户',
                    contents: User.compiledAddTpl.render({'departs':$("table").data('departs')})
                }).showModal();
                $.post('/user/get', {
                    id:node.data('id')
                }, function(data) {
                    var data = JSON.parse(data);
                    console.log(data);
                    $("#radio"+data.sex).attr("checked","checked");
                    $(".name").val(data.name);
                    $(".nick").val(data.nick);
                    $(".phone").val(data.phone);
                    $(".email").val(data.email);
                    $(".avatar").val(data.avatar);
                    if(data.avatar){
                        $(".avatar_show").attr("src",data.avatar).removeClass("hide");
                    }
                    $(".departId").val(data.departId);
                    $(".btn-margin").addClass("btn-margin-edit");
                    $(".btn-margin-edit").data('id',node.data('id'));

                    // $('.avatar_show').Jcrop({
                    //     setSelect:   [ 0, 0, 100, 100 ],
                    //     aspectRatio: 1/1,
                    //     onChange: function(c){
                    //         $(".crop_info").val(JSON.stringify(c));
                    //     }
                    // });
                });
                
            },
            addUserAlert : function(){
                User.addTpl();
                $.fn.SimpleModal({
                    btn_ok: '添加',
                    model: 'confirm',
                    callback: function(node){
                        User.addUser();
                    },
                    overlayClick: false,
                    width: 660,
                    title: '添加用户',
                    contents: User.compiledAddTpl.render({'departs':$("table").data('departs')})
                }).showModal();
            },
            addTpl : function(){
                var tpl =[
                    '<div class="add_user_div" role="form">',
                    '    <div class="clearfix div_20">',
                    '        <input type="text" class="form-control input-sm name pull-left" placeholder="真名">',
                    '        <input type="text" style="margin-left:20px;" class="form-control input-sm nick" placeholder="花名">',
                    '    </div>',
                    '    <div class="clearfix div_20">',
                    '        <input type="password" class="form-control input-sm password" placeholder="密码">',
                    '        <select class="form-control departId input-sm" style="margin-left:20px;">',
                    '            {@each departs as depart}',
                    '                <option value="${depart.id}">${depart.departName}</option>',
                    '            {@/each}',
                    '        </select>',
                    // '        <input type="text" style="margin-left:20px;" class="form-control input-sm position" placeholder="地址">',
                    '    </div>',
                    '    <div class="clearfix div_20">',
                    '        <div class="radio">',
                    '            <span class="text">性别:</span>',
                    '            <label>',
                    '                <input type="radio" id="radio1" class="sex" name="sex[]" value="1" checked="">男',
                    '            </label>',
                    '            <label>',
                    '                <input type="radio" id="radio0" class="sex" name="sex[]" value="0">女',
                    '            </label>',
                    '        </div>',
                    '        <input type="file" style="margin-left:20px;" id="file_avatar" class="file_avatar" name="file_avatar">',
                    '        <input type="hidden" id="avatar" class="avatar" name="avatar">',
                    '    </div>',
                    '    <div class="clearfix div_20">',
                    '        <input type="text" class="form-control input-sm email" placeholder="邮箱">',
                    '        <input type="text" style="margin-left:20px;" class="form-control input-sm phone" placeholder="手机">',
                    '    </div>',
                    '   <div class="hide crop_info"></div>',
                    // '    <div class="clearfix">',
                    // '        <img src="" style="height:100px;margin-left:20px;" class="avatar_show hide">',
                    '        <img src=""  class="avatar_show hide" style="max-width:100px;">',
                    // '       <a href="javascript:;" class="start_upload">上传</a>',
                    // '    </div>',
                    '</div>'
                ].join('\n');
                User.compiledAddTpl = juicer(tpl);
            },
            tpl : function(){
                var tpl=[
                    '{@each users as user}',
                    '   <tr data-id="${user.id}">',
                    '       <td>${user.id}</td>',
                    '       <td>${user.sex}</td>',
                    '       <td>${user.name}</td>',
                    '       <td>${user.nick}</td>',
                    '       <td>${user.phone}</td>',
                    '       <td>${user.email}</td>',
                    '       <td>{@if user.avatar}<img style="height:30px;" src="${user.avatar_value}">{@/if}</td>',
                    '       <td>${user.depart_value}</td>',
                    '       <td><button style="margin-right:10px;" class="btn btn-warning btn-sm edit_user">编辑</button><button href="javascript:;" class="btn btn-danger btn-sm del_user">删除</button></td>',
                    '   </tr>',
                    '{@/each}'
                ].join('\n');
                User.compiledTpl = juicer(tpl);
            },
            upload : function(node){
                var file = $('.file_avatar')[0].files[0];
                var reader = new FileReader();
                reader.onload = function (rResult) {
                    var filename = file.name;
                    var options = {
                        type: 'POST',
                        url: '/user/upload?filename='+filename+"&crop_info="+$(".crop_info").val(),
                        data: reader.result,
                        success:function(result){
                            var data = result;
                            if(data.status == 'success'){
                                $(".avatar_show").attr("src",data.real_path).removeClass("hide");
                                $(".avatar").val(data.file);
                            }
                        },
                        processData: false,
                        contentType: false,
                        dataType:"json"
                    };
                    $.ajax(options);
                };
                reader.readAsArrayBuffer(file);
            }
        }
        $(function(){

            User.getUser();

            $(".next_page,.prev_page").live("click",function(){
                User.getUser($(this).data('page'));
            })

            $(".del_user").live("click",function(){
                User.delUser($(this).parents('tr'));
            })

            $(".add_user").click(function(){
                User.addUserAlert();
            })

            $(".edit_user").live("click",function(){
                User.editUserAlert($(this).parents('tr'));
            })
            $(".file_avatar").live("change",function(){
                User.upload();
            })
            // $(".start_upload").live("click",function(){
            //     User.upload();
            // })
        })
    </script>
    <style>
        .add_user_div .div_20{
            margin-top: 20px;
        }
        .radio span{
            display: inline-block;
            width: 50px;
        }
        .add_user_div div.radio{
            margin: 0;
        }
        .add_user_div input,.radio,.add_user_div select{
            width: 300px;
            float: left;
        }
        .radio input{
            width: 20px;
        }
    </style>
</aside>