// --- BEGIN Global variables -----------------------------------
// --- END Global variables -------------------------------------

// Retrieves the available users.
function getUsers() {
    statusBase = "getting users ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_users";
    url = "http://" + location.host + "/cgi-bin/metorg" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateStatus(statusBase + " done", false);
                    updateStatus("", false);

                    // set values in drop-down list
                    $("#select_users option").remove();
                    $.each(data.users, function(index, user) {
                        $("<option value=\"" + user + "\">" + user + "</option>").appendTo($("#select_users"));
                    });
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Sets given task (if defined) as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentTask(tr) {
    if (tr.length == 0) return;
    $("#table_tasks tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row
}

// Handles selecting a row in the task table.
// tr is the jQuery selector for the corresponding <tr> element.
function selectTask(tr) {
    setCurrentTask(tr);
}

$(document).ready(function() {

    // --- TABLESORTER -----------------------

    initTablesorter();

    var options = {
        widthFixed : false,
        showProcessing: true,
        // headerTemplate : '{content} {icon}', // Add icon for jui theme; new in v2.7!

        // widgets: [ 'uitheme', 'zebra', 'stickyHeaders', 'filter' ],
        widgets: [ 'uitheme', 'zebra', 'stickyHeaders' ],

        widgetOptions: {

            // extra class name added to the sticky header row
            stickyHeaders : '',
            // number or jquery selector targeting the position:fixed element
            stickyHeaders_offset : 0,
            // added to table ID, if it exists
            stickyHeaders_cloneId : '-sticky',
            // trigger "resize" event on headers
            stickyHeaders_addResizeEvent : true,
            // if false and a caption exist, it won't be included in the sticky header
            stickyHeaders_includeCaption : true,
            // The zIndex of the stickyHeaders, allows the user to adjust this to their needs
            stickyHeaders_zIndex : 2,
            // jQuery selector or object to attach sticky header to
            stickyHeaders_attachTo : null,

            // adding zebra striping, using content and default styles
            // - the ui css removes the background from default
            // even and odd class names included for this demo to allow switching themes
            zebra   : ["ui-widget-content even", "ui-state-default odd"],
            // use uitheme widget to apply default jquery ui (jui) class names
            // see the uitheme demo for more details on how to change the class names
            uitheme : 'jui'
        }
    };

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_tasks';
    $("#table_tasks").tablesorter(options);
    $(document).on("click", ".tr_tasks td", function(e) {
        selectTask($(e.target).parent());
    });

    $( "#datepicker" ).datepicker({
    	inline: true,
    	dateFormat: "yy-mm-dd"
    });

    $("#datepicker").datepicker("setDate", new Date()); // initialize to current date

    getUsers();
});
