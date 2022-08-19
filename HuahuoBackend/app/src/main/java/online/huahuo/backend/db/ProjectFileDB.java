package online.huahuo.backend.db;

import lombok.Data;
import lombok.NoArgsConstructor;

import javax.persistence.*;
import java.util.Date;

@Entity
@Data
@NoArgsConstructor
@Table(name = "PROJECTS", indexes = {
        @Index(columnList = "name"),
        @Index(columnList = "createdBy")
})
public class ProjectFileDB {
    @Id
    @Column(name = "id", nullable = false)
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    private String name;
    private String type;
    private String version;

    private String createdBy;

    @Column(unique = true)
    private String fullPath;
    private String checksum;

    private Date createTime;
    private Date modifiedTime;

    @Column(nullable = false)
    private ProjectStatus status;

    public ProjectFileDB(String name, String type, String version, String createdBy,
                         String fullPath, String checksum){
        this.name = name;
        this.type = type;
        this.version = version;
        this.fullPath = fullPath;
        this.checksum = checksum;
        this.createdBy = createdBy;
        this.createTime = new Date();
        this.modifiedTime = new Date();
        this.status = ProjectStatus.ACTIVE;
    }
}
