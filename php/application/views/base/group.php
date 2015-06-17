<aside class="right-side">
    <section class="content-header">
        <h1>
            群组管理
            <small>欢迎来到TeamTalk</small>
        </h1>
        <ol class="breadcrumb">
            <li><a href="/"><i class="fa fa-dashboard"></i> 首页</a></li>
            <li class="active">群组管理</li>
        </ol>
    </section>
    <section class="content">
        <button style="margin-bottom:10px;" class="btn btn-primary btn-sm add_group pull-right">新增</button>
        <table class="table table-bordered">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>群名称</th>
                    <th>群头像</th>
                    <!-- <th>群描述</th> -->
                    <th>成员个数</th>
                    <th>编辑</th>
                </tr>
            </thead>
            <tbody>
            </tbody>
        </table>
    </section>
    <script>
        var Group = {
            compiledTpl : null,
            compiledAddTpl : null,
            getgroup : function(page){
                $.getJSON('/group/all', function(data) {
                    $("table").data('group',data);
                    Group.tpl();
                    var _tpl = Group.compiledTpl.render(data);
                    $("tbody").html(_tpl);
                });
            },
            delgroup : function(node){
                $.post('/group/del', {id: node.data('id')}, function(data) {
                    if($.trim(data) == 'success'){
                        node.fadeOut();
                    }
                });
            },
            addgroupAlert : function(){
                Group.addTpl();
                $.fn.SimpleModal({
                    btn_ok: '添加',
                    model: 'confirm',
                    callback: function(node){
                        Group.addgroup();
                    },
                    overlayClick: false,
                    width: 660,
                    title: '添加群组',
                    contents: Group.compiledAddTpl.render({groups:$("table").data('pgroup')})
                }).showModal();
            },
            editgroupAlert : function(node){
                Group.addTpl();
                $.fn.SimpleModal({
                    btn_ok: '编辑',
                    model: 'confirm',
                    callback: function(node){
                        Group.editgroup();
                    },
                    overlayClick: false,
                    width: 660,
                    title: '编辑群组',
                    contents: Group.compiledAddTpl.render({groups:$("table").data('pgroup')})
                }).showModal();
                $.post('/group/get', {
                    id:node.data('id')
                }, function(data) {
                    var data = JSON.parse(data);
                    $(".name").val(data.name);
                    $(".avatar").val(data.avatar);
                    $(".adesc").val(data.adesc);
                    if(data.avatar){
                        $(".avatar_show").attr("src",data.avatar_value).removeClass("hide");
                    }
                    if(data.pid == 0){
                        $(".pid").attr("disabled","disabled");
                    }else{
                        $($(".pid option")[0]).attr("disabled","disabled");
                    }
                    $(".btn-margin").addClass("btn-margin-edit");
                    $(".btn-margin-edit").data('id',node.data('id'));
                });
            },
            editgroupMemberAlert : function(node){
                window.checked = [];
                Group.editMemberTpl();
                $.fn.SimpleModal({
                    btn_ok: '编辑',
                    hideFooter: true,
                    model: 'modal',
                    overlayClick: false,
                    width: 660,
                    title: '编辑群成员',
                    contents: Group.compiledEditMemberTpl.render()
                }).showModal();
                $("table").data('id',node.data('id'));
                $.post('/group/getMember', {
                    id:node.data('id')
                }, function(data) {
                    var data = JSON.parse(data);
                    var users = data.users;
                    for(i in users){
                        window.checked.push(users[i].userId);
                        $(".choosed_member .memberList").append('<li class="member" data-id="'+users[i].userId+'">'+users[i].name+'<span class="remove_member">x</span></li>')
                    }
                    Group.getUser();
                });
            },
            editgroup : function(){
                $.post('/group/edit', {
                    name: $(".name").val(),
                    avatar: $(".avatar").val(),
                    adesc: $(".adesc").val(),
                    id:$(".btn-margin-edit").data('id')
                }, function(data) {
                    if($.trim(data) == 'success'){
                        $.fn.hideModal();
                        $(".add_group").before('<div class="alert alert-success alert-dismissable"><i class="fa fa-check"></i><button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>编辑成功</div>')
                        setTimeout(function(){
                            $(".alert").remove();
                        },3000);
                        Group.getgroup();
                    }else{
                        $(".btn-margin-edit").text('编辑失败');
                    }
                });
            },
            addgroup : function(){
                $.post('/group/add', {
                    name: $(".name").val(),
                    // avatar: $(".avatar").val(),
                    // adesc: $(".adesc").val()
                }, function(data) {
                    if($.trim(data) == 'success'){
                        $.fn.hideModal();
                        $(".add_group").before('<div class="alert alert-success alert-dismissable"><i class="fa fa-check"></i><button type="button" class="close" data-dismiss="alert" aria-hidden="true">×</button>添加成功</div>')
                        Group.getgroup();
                    }else{
                        $(".btn-group").text('添加失败');
                    }
                });
            },
            editMemberTpl : function(){
                var tpl =[
                    '<div class="choosed_member">',
                    '   <ul class="clearfix memberList">',
                    '   </ul>',
                    '</div>',
                    '<div class="unchoosed_member">',
                    '   <ul class="clearfix memberList">',
                    '   </ul>',
                    '   <ul class="pagination">',
                    '       <li class="prev_page">',
                    '           <a href="javascript:;">上一页</a>',
                    '       </li>',
                    '       <li class="show_page">',
                    '           <a href="javascript:;"></a>',
                    '       </li>',
                    '       <li class="next_page">',
                    '           <a href="javascript:;">下一页</a>',
                    '       </li>',
                    '   </ul>',
                    '</div>'
                ].join('\n');
                Group.compiledEditMemberTpl = juicer(tpl);
            },
            addTpl : function(){
                var tpl =[
                    '<div class="add_group_div" role="form">',
                    '   <input type="text" class="form-control input-sm name" placeholder="群名称">',
                    // '   <input type="text" class="form-control input-sm adesc" placeholder="群描述">',
                    // '   <input type="hidden" class="form-control input-sm avatar" placeholder="群头像">',
                    // '   <input type="file" class="input-sm file_avatar pull-left" style="width:310px;">',
                    '   <img src="" style="height: 50px;float: left;margin-top: 10px;" class="avatar_show hide" >',
                    '</div>'
                ].join('\n');
                Group.compiledAddTpl = juicer(tpl);
            },
            tpl : function(){
                var tpl=[
                    '{@each groups as group}',
                    '   <tr data-id="${group.id}">',
                    '       <td>${group.id}</td>',
                    '       <td>${group.name}</td>',
                    '       <td>{@if group.avatar}<img style="height:30px;" src="${group.avatar_value}">{@/if}</td>',
                    '       <td>${group.userCnt}</td>',
                    '       <td><button style="margin-right:10px;" class="btn btn-warning btn-sm edit_group hide">编辑</button><button style="margin-right:10px;" class="btn btn-danger btn-sm del_group hide">删除</button><button class="btn btn-info btn-sm edit_group_member">编辑群成员</button></td>',
                    '   </tr>',
                    '{@/each}'
                ].join('\n');
                Group.compiledTpl = juicer(tpl);
            },
            upload : function(node){
                var file = $('.file_avatar')[0].files[0];
                var reader = new FileReader();
                reader.onload = function (rResult) {
                    var filename = file.name;
                    var options = {
                        type: 'POST',
                        url: '/user/upload?filename='+filename,
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
            },
            getUser : function(page){
                if(!page){
                    page = 0;
                }
                $.getJSON('/user/all', {
                    start : page
                }, function(data) {
                    var users = data.users;
                    $(".unchoosed_member .memberList").html('');
                    for(i in users){
                        console.log(window.checked);
                        console.log(users[i].id+'');
                        if($.inArray(users[i].id+'',window.checked) == -1){
                            $(".unchoosed_member .memberList").append('<li class="member" disabled data-id="'+users[i].id+'">'+users[i].name+'</li>');
                        }
                    }
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
            addMember : function(node,id){
                $.post('/group/changeMember', {
                    id : id,
                    userId  : node.data('id'),
                    count   : $(".choosed_member .memberList li").length-0+1,
                    change  : 1
                }, function(data) {
                    var userId = node.data('id');
                    var name = node.text();
                    $(".choosed_member .memberList").append('<li class="member" data-id="'+userId+'">'+name+'<span class="remove_member">x</span></li>');
                    node.remove();
                });
            },
            removeMember : function(node,id){
                $.post('/group/changeMember', {
                    id : id,
                    userId  : node.data('id'),
                    count   : $(".choosed_member .memberList li").length-0-1,
                    change  : 2
                }, function(data) {
                    node.remove();
                });
            }
        }
        $(function(){
            Group.getgroup();

            $(".del_group").live("click",function(){
                Group.delgroup($(this).parents('tr'));
            })

            $(".add_group").click(function(){
                Group.addgroupAlert();
            })

            $(".edit_group").live("click",function(){
                Group.editgroupAlert($(this).parents('tr'));
            })

            $(".file_avatar").live("change",function(){
                Group.upload($(this));
            })

            $(".edit_group_member").live("click",function(){
                Group.editgroupMemberAlert($(this).parents('tr'));
            })

            $(".next_page,.prev_page").live("click",function(){
                Group.getUser($(this).data('page'));
            })

            $(".unchoosed_member .member").live("click",function(){
                Group.addMember($(this),$("table").data('id'));
            })

            $(".remove_member").live("click",function(){
                Group.removeMember($(this).parent(),$("table").data('id'));
            })
        })
    </script>
    <style>
        input{
            margin-bottom: 10px;
        }
        .choosed_member,.unchoosed_member{
            width: 620px;
            height: 100px;
        }
        .unchoosed_member{
            height: 200px;
            border-top: 1px solid #ddd;
        }
        .memberList{
            margin: 0;
            padding: 0;
        }
        .choosed_member li.member,.unchoosed_member li.member{
            list-style: none;
            float: left;
            border-radius: 2px;
            border: 1px solid #ddd;
            padding: 1px 5px;
            margin: 5px;
            position: relative;
            cursor: pointer;
        }
        .remove_member{
            background: red;
            font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
            font-size: 11px;
            text-indent: 2px;
            width: 10px;
            height: 10px;
            position: absolute;
            top: -5px;
            color: white;
            line-height: 10px;
            border-radius: 5px;
            cursor: pointer;
        }
        .pagination{
            margin-top: 20px;
        }
        .pagination li a{
            padding: 5px;
        }
    </style>
</aside>