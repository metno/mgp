// --- BEGIN Global variables -----------------------------------
// --- END Global variables -------------------------------------

// Retrieves the available local boards.
function getLocalBoards() {
    statusBase = "getting local boards ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_backedup_boards";
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

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

                    // load table
                    clearTable("#table_lboards");
		    boards = data.boards;
                    html = "";
                    for (i = 0; i < boards.length; ++i) {
                        html += "<tr class=\"tr_lboards\">";
                        html += "<td>" + boards[i].id + "</td>";
                        html += "<td>" + boards[i].name + "</td>";
                        html += "</tr>";
                    }

                    $("#table_lboards > tbody:last").append(html);
                    $("#table_lboards").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_lboards").trigger("appendCache");


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


// Handles selecting a row in the table of local boards.
// tr is the jQuery selector for the corresponding <tr> element.
function selectLocalBoard(tr) {
    alert('selectLocalBoard() ...');
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

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_lboards';
    $("#table_lboards").tablesorter(options);
    $(document).on("click", ".tr_lboards td", function(e) {
        selectLocalBoard($(e.target).parent());
    });

    getLocalBoards();
});
